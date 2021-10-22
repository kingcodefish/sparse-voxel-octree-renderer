#include <cnpy.h>
#include <torch/torch.h>

#include <string>
#include <iostream>

#include <cub/device/device_scan.cuh>

using namespace std;

using point_data = ushort4;
using morton_code = unsigned long long;

/**
 * Conventions:
 * d_ = device
 * h_ = host
 */

/**
 * Does the CUDA kernels fail if the blockDim < 1024? :/
 */

const uint8_t MAX_LEVELS = 8;

static __inline__ __host__ __device__ point_data make_point_data(ushort x, ushort y, ushort z)
{
    point_data p;
    p.x = x;
    p.y = y;
    p.z = z;
    p.w = 0;

    return p;
}

static __inline__ __host__ __device__ point_data to_point(morton_code mcode)
{
    point_data p = make_point_data(0, 0, 0);

    for (int i = 0; i < 16; ++i)
    {
        p.x |= (mcode & (0x1ull << (3 * i + 2))) >> (2 * i + 2);
        p.y |= (mcode & (0x1ull << (3 * i + 1))) >> (2 * i + 1);
        p.z |= (mcode & (0x1ull << (3 * i + 0))) >> (2 * i + 0);
    }

    return p;
}

__global__ void d_scan_nodes(const uint numBytes, const uint8_t* d_octree, uint* d_info)
{
    uint tidx = blockIdx.x * 1024 + threadIdx.x;

    if (tidx < numBytes)
        d_info[tidx] = __popc(d_octree[tidx]);
}

__global__ void d_morton_to_point(const uint psize, morton_code *dataIn, point_data* dataOut)
{
    uint tidx = blockDim.x * blockIdx.x + threadIdx.x;

    if (tidx < psize)
        dataOut[tidx] = to_point(dataIn[tidx]);
}

__global__ void d_nodes_to_morton(const uint psize, const uint8_t* d_octreeData, const uint* d_prefixSum,
                                  const morton_code* d_mDataIn, morton_code* d_mDataOut)
{
    uint tidx = blockIdx.x * 1024 + threadIdx.x;

    if (tidx < psize)
    {
        uint8_t bits = d_octreeData[tidx];
        morton_code code = d_mDataIn[tidx];
        int addr = d_prefixSum[tidx];

        for (int i = 7; i >= 0; --i)
        {
            if (bits & (0x1 << i))
                d_mDataOut[addr--] = 8 * code + i;
        }
    }
}

struct SPC
{
    SPC() {}

    /**
     * Load NPZ file for Structured Point Cloud
     * 
     * What is the structure of this?
     * b0, b1, cc, cf, octree, pyramid, w0, w1
     * Yeah, I'm as clueless as you, but I'm hoping octree
     * is the only useful thing in this.
     * 
     * A note, the original used at::Tensor, but torch::Tensor is identical.
     * 
     * Arguments:
     * path - The path to the .npz file relative to the executable.
     */
    void load_npz(string path)
    {
        // Load in the npz file with CNPY
        cnpy::npz_t file = cnpy::npz_load(path);

        // Pull out the relevant array variables (the NPYs).
        cnpy::NpyArray octreeArray = file["octree"];
        uint8_t* octree = octreeArray.data<uint8_t>();

        // Debugging information
        cout << "Octree Data --------" << endl;
        cout << "Length: " << octreeArray.num_vals << endl;

        // Move the octree data from the CPU to the GPU
        m_octree = torch::zeros({ static_cast<long>(octreeArray.num_vals) }, torch::device(torch::kCUDA).dtype(torch::kByte));
        uint8_t* octreeDest = reinterpret_cast<uint8_t*>(m_octree.data_ptr<uint8_t>());
        cudaMemcpy(octreeDest, octree, octreeArray.num_vals, cudaMemcpyHostToDevice);

        vector<torch::Tensor> tmp;
        tmp = set_geometry(m_octree);
    }

    /**
     * Encode the Morton encoding of the octree to the
     * one-dimensional representation for rendering.
     */
    vector<torch::Tensor> set_geometry(torch::Tensor octree)
    {
        uint8_t* octreeData = octree.data_ptr<uint8_t>();
        m_osize = octree.size(0);

        m_info = torch::zeros({ m_osize + 1 }, torch::device(torch::kCUDA).dtype(torch::kInt32));
        m_prefixSum = torch::zeros({ m_osize + 1 }, torch::device(torch::kCUDA).dtype(torch::kInt32));
        torch::Tensor pyramidData = torch::zeros({ 2, MAX_LEVELS + 2 }, torch::device(torch::kCPU).dtype(torch::kInt32));
        
        uint* d_info = reinterpret_cast<uint*>(m_info.data_ptr<int>());
        uint* d_prefixSum = reinterpret_cast<uint*>(m_prefixSum.data_ptr<int>());
        int* h_pyramid = pyramidData.data_ptr<int>();

        void* d_temp_storage = nullptr;
        size_t temp_storage_bytes = 0;
        cub::DeviceScan::ExclusiveSum(d_temp_storage, temp_storage_bytes, d_info, d_prefixSum, m_osize + 1);

        torch::Tensor temp_storage = torch::zeros({ (long)temp_storage_bytes }, torch::device(torch::kCUDA).dtype(torch::kByte));
        d_temp_storage = (void*)temp_storage.data_ptr<uint8_t>();

        // Compute exclusive sum 1 element beyond end of list to get inclusive sum starting at d_prefixSum + 1.
        // Adding 1023 before dividing the block size ensures that we use at least 1 block.
        d_scan_nodes<<<(m_osize + 1023) / 1024, 1024>>>(m_osize, octreeData, d_info);
        cub::DeviceScan::ExclusiveSum(d_temp_storage, temp_storage_bytes, d_info, d_prefixSum, m_osize + 1);

        uint psize = 0;
        cudaMemcpy(&psize, d_prefixSum + m_osize, sizeof(int), cudaMemcpyDeviceToHost);
        psize++; // Plus one for root?

        torch::Tensor points = torch::zeros({ psize, 4 }, torch::device(torch::kCUDA).dtype(torch::kInt16));
        point_data* pdata = reinterpret_cast<point_data*>(points.data_ptr<short>());

        torch::Tensor mortons = torch::zeros({ psize }, torch::device(torch::kCUDA).dtype(torch::kInt64));
        morton_code* mdata = reinterpret_cast<morton_code*>(mortons.data_ptr<long>());

        int* pyramid = h_pyramid;
        int* pyramidSum = h_pyramid + MAX_LEVELS + 2;

        uint* S = d_prefixSum + 1; // This shouldn't matter?
        morton_code* M = mdata;
        uint8_t* O = octreeData;

        morton_code m0 = 0;
        cudaMemcpy(M, &m0, sizeof(morton_code), cudaMemcpyHostToDevice);

        int lsize = 1;
        uint currSum, prevSum = 0;

        uint sum = pyramid[0] = lsize;
        pyramidSum[0] = 0;
        pyramidSum[1] = sum;

        int level = 0;
        while (sum <= m_osize)
        {
            d_nodes_to_morton<<<(lsize + 1023) / 1024, 1024>>>(lsize, O, S, M, mdata);
            O += lsize;
            S += lsize;
            M += lsize;

            cudaMemcpy(&currSum, d_prefixSum + prevSum + 1, sizeof(int), cudaMemcpyDeviceToHost);

            lsize = currSum - prevSum;
            prevSum = currSum;

            pyramid[++level] = lsize;
            sum += lsize;
            pyramidSum[level + 1] = sum;
        }

        uint totalPoints = pyramidSum[level + 1];

        d_morton_to_point<<<(totalPoints + 1023) / 1024, 1024>>>(totalPoints, mdata, pdata);
        cudaGetLastError();

        // Assemble output tensors
        std::vector<torch::Tensor> result;
        result.push_back(points);
        result.push_back(pyramidData.index({
            torch::indexing::Slice(torch::indexing::None),
            torch::indexing::Slice(torch::indexing::None,
            level + 2)
        }).contiguous());

        cudaDeviceSynchronize();
        cudaError_t err = cudaGetLastError();

        if (err != cudaSuccess)
        {
            printf("CUDA Error: %s\n", cudaGetErrorString(err));
        }

        return result;
    }

private:
    torch::Tensor m_octree;
    torch::Tensor m_points;
    torch::Tensor m_info;
    torch::Tensor m_prefixSum;
    torch::Tensor m_pyramid;

    uint8_t       m_level = 0;
    uint8_t       m_psize = 0;
    uint8_t       m_osize = 0;
};

int main()
{
    SPC* spc = new SPC();
    spc->load_npz("armadillo.npz");
}