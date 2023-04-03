#pragma once
#include"AABB.h"
#include"Triangle.h"
#include"Ray.h"
#include<queue>
#include <bit>
#include <bitset>
#include <cstdint>
struct Int2 {
	int x;
	int y;
	Int2(int _x, int _y) {
		x = _x;
		y = _y;
	}
};
class bvhNode {
public:
	int leftchild;
	int rightchild;
	int parent;
	AABB bound;
	int flag;
	bool isleaf;
	int leafPrimitiveId;
	bvhNode() {
		leftchild = -1;
		rightchild = -1;
		parent = -1;
		bound = AABB();
		flag = 0;
		isleaf = false;
		leafPrimitiveId = -1;
	}
	bvhNode(const bvhNode& node) {
		leftchild = node.leftchild;
		rightchild = node.rightchild;
		parent = node.parent;
		bound = node.bound;
		flag = node.flag;
		isleaf = node.isleaf;
		leafPrimitiveId = node.leafPrimitiveId;
	}
};
class BVH {
private:
	vector<unsigned long long> sortedMC;
	int Delta(const unsigned long long& Morton, int split)
	{
		return ((split >= 0) && (split < leafNum)) ? countl_zero(Morton ^ sortedMC[split]) : -1;
	}
	void AssignLeaf(const vector<Triangle>& primitives) {
		for (int i = leafNum - 1; i < 2 * leafNum - 1; ++i) {
			Nodes[i].isleaf = true;
			Nodes[i].leftchild = -1;
			Nodes[i].rightchild = -1;
			Nodes[i].leafPrimitiveId = i- leafNum+1;
			Nodes[i].bound = AABB(primitives[i- leafNum + 1]);
		}
	}
	AABB GetBounds(int idx) {
		
		int left = Nodes[idx].leftchild;
		int right = Nodes[idx].rightchild;
		if (left == -1 && right == -1) {
			
		}
		else if (left != -1 && right == -1) {
			Nodes[idx].bound = GetBounds(left);
		}
		else if (left == -1 && right != -1) {
			Nodes[idx].bound = GetBounds(right);
		}
		else {
			Nodes[idx].bound = AABB(GetBounds(left), GetBounds(right));
		}
		//cout << "node " << idx << ": " << endl;
		//cout << glm::to_string(Nodes[idx].bound.minBound) << " " << glm::to_string(Nodes[idx].bound.maxBound) << endl;
		return Nodes[idx].bound;
	}
	void RecuUpdateAABB() {
		Nodes[0].bound = GetBounds(0);
	}
	void UpdateInteriorAABB() {
		for (int i = 1; i < leafNum; ++i) {
			int nodeIndex = leafNum + i - 1;
			int parentIndex = Nodes[nodeIndex].parent;
			while ((parentIndex >= 0) && (parentIndex < leafNum - 1))
			{
				if (Nodes[parentIndex].flag == 0) {
					Nodes[parentIndex].flag = 1;
					break;
				}
				else if (Nodes[parentIndex].flag == 1) {
					int left = Nodes[parentIndex].leftchild;
					int right = Nodes[parentIndex].rightchild;
					const auto& leftAABB = Nodes[left].bound;
					const auto& rightAABB = Nodes[right].bound;
					Nodes[parentIndex].bound = AABB(leftAABB, rightAABB);
					Nodes[parentIndex].flag = 2;
					nodeIndex = parentIndex;

					parentIndex = Nodes[nodeIndex].parent;
				}
				else {
					throw std::runtime_error("BV-Tree updating Interior AABB error");
				}
			}
		}
	}
	int FindSplit(int first, int last) {
		unsigned long long firstCode = sortedMC[first];
		unsigned long long lastCode = sortedMC[last];

		if (firstCode == lastCode)
			return (first + last) >> 1;

		// Calculate the number of highest bits that are the same
		// for all objects, using the count-leading-zeros intrinsic.

		int commonPrefix = countl_zero(firstCode ^ lastCode);

		// Use binary search to find where the next bit differs.
		// Specifically, we are looking for the highest object that
		// shares more than commonPrefix bits with the first one.

		int split = first; // initial guess
		int step = last - first;

		do
		{
			step = (step + 1) >> 1; // exponential decrease
			int newSplit = split + step; // proposed new position

			if (newSplit < last)
			{
				auto& splitCode = sortedMC[newSplit];
				int splitPrefix = countl_zero(firstCode ^ splitCode);
				if (splitPrefix > commonPrefix)
					split = newSplit; // accept proposal
			}
		} while (step > 1);

		return split;
	}
	Int2 DetermineRange(int nodeIndex)
	{
		auto& morton0 = sortedMC[nodeIndex];

		int commonPrefix_L = countl_zero(morton0 ^ sortedMC[nodeIndex - 1]);
		int commonPrefix_R = countl_zero(morton0 ^ sortedMC[nodeIndex + 1]);

		//!	Determine direction of the range (+1 or -1).
		int dir = (commonPrefix_L > commonPrefix_R) ? -1 : 1;

		//!	Compute upper bound for the length of the range.
		int minPrefix = (commonPrefix_L < commonPrefix_R) ? commonPrefix_L : commonPrefix_R;

		int lmax = 2;

		while (Delta(morton0, nodeIndex + lmax * dir) > minPrefix)
		{
			lmax = lmax * 2;
		}

		//!	Find the other end using binary search.
		int l = 0;

		for (int t = lmax / 2; t >= 1; t /= 2)
		{
			if (Delta(morton0, nodeIndex + (l + t) * dir) > minPrefix)
			{
				l = l + t;
			}
		}

		int lastIndex = nodeIndex + l * dir;

		if (lastIndex < nodeIndex)
			return Int2(lastIndex, nodeIndex);
		else
			return Int2(nodeIndex, lastIndex);
	}
	void BuildBVH() {
		// Construct internal nodes.
		for (int idx = 0; idx < leafNum - 1; idx++) // in parallel
		{
			// Find out which range of objects the node corresponds to.
			// (This is where the magic happens!)
			int first, last;
			if (idx == 0) {
				first = 0;
				last = leafNum - 1;
			}
			else {
				Int2 range = DetermineRange(idx);
				first = range.x;
				last = range.y;
			}
			
			//cout <<"id: "<<idx<< " range: " << first << " " << last << " ;split: ";
			// Determine where to split the range.

			int split = FindSplit(first, last);
			//std::cout << split ;
			// Select childA.

			int childA;
			if (split == first)
				childA = leafNum - 1 + split;
			else
				childA = split;

			// Select childB.

			int childB;
			if (split + 1 == last)
				childB = leafNum + split;
			else
				childB = split + 1;

			// Record parent-child relationships.

			Nodes[idx].leftchild = childA;
			Nodes[idx].rightchild = childB;
			if (Nodes[childA].parent != -1 || Nodes[childB].parent != -1) {
				throw std::runtime_error("BV-Tree build error");
			}
			Nodes[childA].parent = idx;
			Nodes[childB].parent = idx;
			//cout << " child: " << childA << " " << childB << endl;
		}
		/*for (int i = 0; i < Nodes.size(); ++i) {
			cout << i << " " << Nodes[i].parent << endl;
		}*/
	}
	
public:
	unsigned int leafNum;
	vector<bvhNode> Nodes;
	vector<vector<int>> leafmap;
	// build bvh tree
	BVH() {
		leafNum = 0;
		sortedMC.resize(0);
		Nodes.resize(0);
	}
	void BuildTree(const vector<Triangle>& primitives) {
		vector<Triangle> leafPrimitives;
		for (int i = 0; i < primitives.size(); ++i) {
			int curid = leafPrimitives.size() - 1;
			if (curid < 0) {
				leafPrimitives.push_back(primitives[i]);
				vector<int> pid;
				pid.push_back(i);
				leafmap.push_back(pid);
			}
			else if (leafPrimitives[curid].mortonCode != primitives[i].mortonCode) {
				leafPrimitives.push_back(primitives[i]);
				vector<int> pid;
				pid.push_back(i);
				leafmap.push_back(pid);
			}
			else {
				leafmap[curid].push_back(i);
			}
		}
		
		leafNum = leafPrimitives.size();
		for (int i = 0; i < leafNum; ++i) {
			sortedMC.push_back(leafPrimitives[i].mortonCode);
		}
		Nodes.resize(2 * leafNum - 1);
		AssignLeaf(leafPrimitives);
		BuildBVH();
		//UpdateInteriorAABB();
		RecuUpdateAABB();
	}
	Intersection IntersectBVH(const Ray& ray, const vector<Triangle>& primitives) const{
		Intersection inter;
		queue<int> nodesQueqe;
		nodesQueqe.push(0);
		while (!nodesQueqe.empty()) {
			int curnodeid = nodesQueqe.front();
			nodesQueqe.pop();
			auto& curNode = Nodes[curnodeid];
			glm::vec3 invDir = glm::vec3(1.0f / ray.dir.x, 1.0f / ray.dir.y, 1.0f / ray.dir.z);
			std::array<int, 3> dirIsNeg;
			dirIsNeg[0] = ray.dir.x > 0 ? 0 : 1;
			dirIsNeg[1] = ray.dir.y > 0 ? 0 : 1;
			dirIsNeg[2] = ray.dir.z > 0 ? 0 : 1;
			if (curNode.bound.IntersectAABB(ray, invDir, dirIsNeg)) {
				if (curNode.isleaf == false) {
					nodesQueqe.push(curNode.leftchild);
					nodesQueqe.push(curNode.rightchild);
				}
				else {
					int tid = curNode.leafPrimitiveId;
					for (int i = 0; i < leafmap[tid].size(); ++i) {
						auto curTriangle = primitives[leafmap[tid][i]];
						
						Intersection curinter = curTriangle.Intersect(ray);
						//cout <<"BVH "<< leafmap[tid][i] << ": " << curinter.happened << " ";
						if (curinter.happened && curinter.distance < inter.distance) {
							inter = curinter;
						}
					}
					
				}
			}
		}
		return inter;
	}
	
};