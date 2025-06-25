# README

## Overview
This is the implementation for Reimagining cryptocurrency transaction pools for lightweight clients and IoT devices

The transaction pool plays a critical role in processing and disseminating transactions in cryptocurrency networks. However, increasing transaction loads strain the resources of full node deployments. We present Neonpool, an innovative transaction pool optimization using bloom filter variants, which reduces the memory footprint of the transaction pool to a fraction. Implemented in C++ and benchmarked using a unique Bitcoin and Ethereum dataset, our solution verifies and forwards transactions with over 99.99\% accuracy and does not necessitate a hard fork. Neonpool is ideally suited for lightweight cryptocurrency clients and for resource-constrained devices such as browsers, systems-on-a-chip, mobile or IoT devices. 

##Preprint
https://www.arxiv.org/abs/2412.16217

##Neonpool
This package contains the source code for two separate projects: **Neonpool-BTC** and **Neonpool-ETH**. Each folder contains implementations specific to Bitcoin (BTC) and Ethereum (ETH) transaction pool.

## Folder Structure

1. **Neonpool-BTC**: Contains source code for Bitcoin transaction processing.
2. **Neonpool-ETH**: Contains source code for Ethereum transaction processing.

## Dataset
1. **Neonpool-BTC**: https://www.kaggle.com/datasets/mempoolstate/mempool-state-bitcoin
2. **Neonpool-ETH**: https://www.kaggle.com/datasets/txpoolstate/txpool-state

## Included Libraries

The package comes with the following supporting libraries:
- **RapidJSON**: For efficient JSON parsing and manipulation.
- **libbf**: For bloom filter implementation.
- **cppdatetimelite**: For lightweight datetime operations.
- **Helpers Library**: Provides utility functions and transaction structure support for Bitcoin and Ethereum.

## Code Portability

The provided source code is portable and can be adapted to different environments. To run the code:
1. Update the paths to your data files and project directories to match your environment.
2. Ensure all dependencies are correctly configured.

## How to Use 

1. Choose the appropriate folder (Neonpool-BTC or Neonpool-ETH) based on the target cryptocurrency.
2. Select the desired project for your use case (Hourly Resetting Filters or Decaying Bloom Filters).
3. Modify the project settings and paths as needed for your environment.
4. Compile and run the project.

For additional information or troubleshooting, refer to the code comments.

## Note

Please ensure that all dependencies are installed and configured before running the projects. If you encounter issues, verify the environment setup and paths are correctly aligned with your system.
