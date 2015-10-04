/*
 * Tree.cpp
 *
 *  Created on: Mar 24, 2015
 *      Author: Tadeze
 */

#include "Tree.hpp"

bool Tree::useVolumeForScore = false;

void Tree::iTree(std::vector<int> const &dIndex, const doubleframe *dt, int &maxheight){
	this->nodeSize = dIndex.size(); // Set size of the node
	if (dIndex.size() <= 1 || (maxheight > 0 && this->depth > maxheight))
		return;

	double min, max, temp;
	int attCnt = dt->ncol, attribute, rIdx;
	int *attPool = new int[attCnt];
	for(int i = 0; i < attCnt; ++i)
		attPool[i] = i;
	while(attCnt > 0){
		rIdx = randomI(0, attCnt - 1);
		attribute = attPool[rIdx];
		min = max = dt->data[dIndex[0]][attribute];
		for(unsigned int i = 1; i < dIndex.size(); ++i){
			temp = dt->data[dIndex[i]][attribute];
			if (min > temp) min = temp;
			if (max < temp) max = temp;
		}
		if(min < max) break; // found attribute
		--attCnt;
		attPool[rIdx] = attPool[attCnt];
	}
	delete []attPool;
	if(attCnt <= 0) return; // no valid attribute found

	this->minAttVal = min;
	this->maxAttVal = max;
	this->splittingAtt = attribute;
	this->splittingPoint = randomD(min, max);

	//Split the node into two
	std::vector<int> lnodeData, rnodeData;
	for (unsigned int i = 0; i < dIndex.size(); i++) {
		temp = dt->data[dIndex[i]][this->splittingAtt];
		if (temp <= this->splittingPoint && temp != max)
			lnodeData.push_back(dIndex[i]);
		else
			rnodeData.push_back(dIndex[i]);
	}

	leftChild = new Tree();
	leftChild->parent = this;
	leftChild->depth = this->depth + 1;
	leftChild->volume = this->volume + log(this->splittingPoint - this->minAttVal)
									 - log(this->maxAttVal - this->minAttVal);
	leftChild->iTree(lnodeData, dt, maxheight);

	rightChild = new Tree();
	rightChild->parent = this;
	rightChild->depth = this->depth + 1;
	rightChild->volume = this->volume + log(this->maxAttVal - this->splittingPoint)
									  - log(this->maxAttVal - this->minAttVal);
	rightChild->iTree(rnodeData, dt, maxheight);
}

/*
 * takes an instance as vector of double
 */
double Tree::pathLength(double *inst) {
	Tree *cur = this;
	while(cur->leftChild != NULL || cur->rightChild != NULL){
		if(cur->nodeSize <= 1) break;
		if (inst[cur->splittingAtt] <= cur->splittingPoint)
			cur = cur->leftChild;
		else
			cur = cur->rightChild;
	}
	if(Tree::useVolumeForScore == true){
		if(cur->nodeSize <= 1)
			return (-cur->nodeSize/cur->volume);
		return (-1/(cur->volume - log(cur->newNodeSize)));
	}
	return (cur->depth + avgPL(cur->nodeSize));
}

// for online IF
void Tree::renewNodeSize(){
	nodeSize = newNodeSize;
	newNodeSize = 0;
	if(leftChild == NULL && rightChild == NULL)
		return;
	leftChild->renewNodeSize();
	rightChild->renewNodeSize();
}

void Tree::update(const double inst[]){
	++newNodeSize;
	if(leftChild == NULL && rightChild == NULL)
		return;
	if(inst[splittingAtt] <= splittingPoint)
		leftChild->update(inst);
	else
		rightChild->update(inst);
}

void Tree::printDepthAndNodeSize(std::ofstream &out){
	if(this->leftChild == NULL && this->rightChild == NULL){
		out << this->depth << "," << this->nodeSize << std::endl;
	}else{
		this->leftChild->printDepthAndNodeSize(out);
		this->rightChild->printDepthAndNodeSize(out);
	}
}
