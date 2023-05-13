#include "ParticalSystem.h"
#include <iostream>
#include "Global.h"
#include <unordered_set>

namespace Fluid2d {

	ParticalSystem::ParticalSystem() {

	}

	ParticalSystem::~ParticalSystem() {

	}

	void ParticalSystem::SetContainerSize(glm::vec2 corner, glm::vec2 size) {
		mLowerBound = corner;
		mUpperBound = corner + size;

		mBlockRowNum = floor(size.y / mSupportRadius);
		mBlockColNum = floor(size.x / mSupportRadius);
		mBlockSize = glm::vec2(size.x / mBlockColNum, size.y / mBlockRowNum);
	}

	int32_t ParticalSystem::AddFluidBlock(glm::vec2 corner, glm::vec2 size, glm::vec2 v0) {
		glm::vec2 blockLowerBound = corner;
		glm::vec2 blockUpperBound = corner + size;

		if (blockLowerBound.x < mLowerBound.x ||
			blockLowerBound.y < mLowerBound.y ||
			blockUpperBound.x > mUpperBound.x ||
			blockUpperBound.y > mUpperBound.y) {
			return -1;
		}

		int width = size.x / mParticalDiameter;
		int height = size.y / mParticalDiameter;

		std::vector<glm::vec2> position(width * height);
		std::vector<glm::vec2> velocity(width * height, v0);
		std::vector<glm::vec2> accleration(width * height, glm::vec2(0.0f, 0.0f));

		int p = 0;
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				position[p] = corner + glm::vec2(j * mParticalDiameter * 0.6 + mParticalRadius, i * mParticalDiameter * 0.6 + mParticalRadius);
				if (i % 2) {
					position[p].x += mParticalRadius;
				}
				p++;
			}
		}

		mPositions = position;
		mVelocity = velocity;
		mAccleration = accleration;
		return 0;

	}

	void ParticalSystem::SearchNeighbors() {
		Glb::Timer timer;
		timer.Start();
		// 分块
		BuildBlockStructure();
		std::cout << "block time = " << timer.GetTime() << std::endl;

		timer.Start();
		mNeighbors = std::vector<std::vector<NeighborInfo>>(mPositions.size(), std::vector<NeighborInfo>(0));

		for (int i = 0; i < mPositions.size(); i++) {	// 对所有粒子查找邻居
			glm::vec2 deltePos = mPositions[i] - mLowerBound;
			uint32_t bc = floor(deltePos.x / mBlockSize.x);
			uint32_t br = floor(deltePos.y / mBlockSize.y);

			int bIdi = GetBlockIdByPosition(mPositions[i]);

			// 对所有相邻Block
			for (int dr = -1; dr <= 1; dr++) {
				for (int dc = -1; dc <= 1; dc++) {
					if (bc + dc < 0 || bc + dc >= mBlockColNum) {
						continue;
					}

					if (br + dr < 0 || br + dr >= mBlockRowNum) {
						continue;
					}


					int bIdj = bIdi + dr * mBlockColNum + dc;
					std::vector<int>& block = mBlocks[bIdj];
					for (int j : block) {	// 对Block中所有粒子
						if (i == j) {
							continue;
						}
						NeighborInfo nInfo{};
						nInfo.radius = mPositions[i] - mPositions[j];
						nInfo.distance = glm::length(nInfo.radius);
						nInfo.index = j;
						if (nInfo.distance <= mSupportRadius) {
							mNeighbors[i].push_back(nInfo);
						}
					}
				}
			}
		}
		std::cout << "fn time = " << timer.GetTime() << std::endl;
		//int p = 0;
		//for (int i = 0; i < 60; i++) {
		//	for (int j = 0; j < 60; j++) {
		//		std::cout << mNeighbors[p].size() << " ";
		//		p++;
		//	}
		//	std::cout << std::endl;
		//}

	}

	size_t ParticalSystem::GetBlockIdByPosition(glm::vec2 position) {
		if (position.x < mLowerBound.x ||
			position.y < mLowerBound.y ||
			position.x > mUpperBound.x ||
			position.y > mUpperBound.y) {
			return -1;
		}

		glm::vec2 deltePos = position - mLowerBound;
		uint32_t c = floor(deltePos.x / mBlockSize.x);
		uint32_t r = floor(deltePos.y / mBlockSize.y);
		return r * mBlockColNum + c;
	}

	void ParticalSystem::BuildBlockStructure() {
		mBlocks = std::vector<std::vector<int>>(mBlockColNum * mBlockRowNum, std::vector<int>(0));

		for (int i = 0; i < mPositions.size(); i++) {
			int bId = GetBlockIdByPosition(mPositions[i]);
			mBlocks[bId].push_back(i);
		}

		//int p = 0;
		//for (int i = 0; i < mBlockRowNum; i++) {
		//	for (int j = 0; j < mBlockColNum; j++) {
		//		std::cout << mBlocks[p].size() << " ";
		//		p++;
		//	}
		//	std::cout << std::endl;
		//}
	}

}

