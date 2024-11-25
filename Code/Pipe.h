#pragma once

#include <FrameMath/Vector2.h>
#include <FrameMath/ColorMath.h>

#include <FrameCore/Globals.h>
#include <FrameRender/Renderer.h>

#include "Components/Machine/DeviceComponent.h"

#include <vector>

class CEditorDeviceComponent;

struct SPipeNode {
	SPipeNode(const Frame::Vec2 & _pos)
		: pos { _pos }
	{}
	Frame::Vec2 pos;
	SPipeNode * nodes[4] = { nullptr, nullptr, nullptr, nullptr };
	CDeviceComponent * pDevice = nullptr;
	int dirIndexForDevice = -1;
};

struct SEditorPipeNode {
	SEditorPipeNode(const Frame::Vec2 & _pos)
		: pos { _pos }
	{}
	Frame::Vec2 pos;
	SEditorPipeNode * nodes[4] = { nullptr, nullptr, nullptr, nullptr };
	CEditorDeviceComponent * pDevice = nullptr;
	int dirIndexForDevice = -1;
};

void DrawPipeSingleLine(Frame::Vec2 p1, const Frame::Vec2 & p2, Frame::ColorRGB color, float alpha);

template<typename PipeNodeT>
static inline void DrawPipeCross(const PipeNodeT * pPipeNode, const Frame::Vec2 & pos, Frame::ColorRGB color, float alpha, float rotation)  {
	if(!pPipeNode) {
		return;
	}

	Assets::EDeviceStaticSprite colorSprite = Assets::EDeviceStaticSprite::pipe_interface_color;
	Assets::EDeviceStaticSprite baseSprite = Assets::EDeviceStaticSprite::pipe_interface;

	float angle = 0.f;
	
	if(pPipeNode->pDevice) {
		colorSprite = Assets::EDeviceStaticSprite::pipe_interface_color;
		baseSprite = Assets::EDeviceStaticSprite::pipe_interface;
	} else {
		int count = 0;
		for(int i = 0; i < 4; i++) {
			count += pPipeNode->nodes[i] != nullptr;
		}
		if(count == 4) {
			colorSprite = Assets::EDeviceStaticSprite::pipe_cross_color;
			baseSprite = Assets::EDeviceStaticSprite::pipe_cross;
		} else
		if(count == 3) {
			colorSprite = Assets::EDeviceStaticSprite::pipe_junction_color;
			baseSprite = Assets::EDeviceStaticSprite::pipe_junction;
			for(int i = 0; i < 4; i++) {
				if(pPipeNode->nodes[i] == nullptr) {
					angle = static_cast<float>(-(i - 1) * 90);
					break;
				}
			}
		} else
		if(count == 2) {
			colorSprite = Assets::EDeviceStaticSprite::pipe_bend_color;
			baseSprite = Assets::EDeviceStaticSprite::pipe_bend;
			angle = 90.f;
			int i;
			for(i = 0; i < 4; i++) {
				if(pPipeNode->nodes[i] == nullptr && pPipeNode->nodes[i == 3 ? 0 : i + 1] == nullptr) {
					angle = static_cast<float>(-i * 90);
					break;
				}
			}
			if(i == 4) {
				angle = 0.f;
				colorSprite = Assets::EDeviceStaticSprite::pipe_interface_color;
				baseSprite = Assets::EDeviceStaticSprite::pipe_interface;
			}
		}
	}

	const Frame::Vec2 posAdd = pPipeNode->pos.GetRotatedDegree(rotation);
	Frame::gRenderer->DrawSpriteBlended(Assets::GetStaticSprite(colorSprite)->GetImage(), pos + posAdd, color, alpha, 1.f, angle + rotation);
	Frame::gRenderer->DrawSpriteBlended(Assets::GetStaticSprite(baseSprite)->GetImage(), pos + posAdd, 0xFFFFFF, alpha, 1.f, angle + rotation);
}

template<typename PipeNodeT, typename PipeT>
void DrawPipe(const PipeT & pipeNodes, const Frame::Vec2 & pos, Frame::ColorRGB color, float alpha, float rotation) {
	std::unordered_map<const PipeNodeT *, uint8> pipeNodesMark;

	for(auto & pPipeNode : pipeNodes) {
		uint8 markVal = 0;
		for(int i = 0; i < 4; i++) {
			if(pPipeNode->nodes[i]) {
				markVal |= 0b1 << i;
			}
		}
		if(markVal == 0) {
			DrawPipeCross(pPipeNode, pos, color, alpha, rotation);
		}
		pipeNodesMark.insert({ pPipeNode, markVal });
	}

	for(auto & pPipeNode : pipeNodes) {
		for(int i = 0; i < 2; i++) {
			if(!pipeNodesMark[pPipeNode]) {
				break;
			}
			if(!pPipeNode->nodes[i]) {
				continue;
			}
			DrawPipeSingleLine(pos + pPipeNode->pos.GetRotatedDegree(rotation), pos + pPipeNode->nodes[i]->pos.GetRotatedDegree(rotation), color, alpha);

			pipeNodesMark[pPipeNode] &= ~(0b1 << i);
			pipeNodesMark[pPipeNode->nodes[i]] &= ~(0b100 << i);
			if(!pipeNodesMark[pPipeNode]) {
				DrawPipeCross(pPipeNode, pos, color, alpha, rotation);
			}
			if(!pipeNodesMark[pPipeNode->nodes[i]]) {
				DrawPipeCross(pPipeNode->nodes[i], pos, color, alpha, rotation);
			}
		}
	}
}

// 关于 mode：
// 0 : 常规
// 1 : 此次递归是为了擦除这一段管道而运行的，递归会在T字或十字路口停下，并将该节点从路口节点的连接中移除，但是注意，该模式下并不会对内存进行释放（因为有时候要用即将擦除的管道去做一些别的事情）
// 2 : 此次递归会穿透装置，也就是说对管道连接着的装置所连接着的其它管道也会进行递归遍历，最后一个参数填入一个 std::vector<PipeNodeT *> *，用以保存此次递归得到的所有与装置相连的节点
template<typename PipeNodeT>
void PipeRecursion(std::unordered_set<PipeNodeT *> * outSet, PipeNodeT * pNode, int mode, void * pExtraDataForModes = nullptr) {
	if(pNode == nullptr) {
		return;
	}

	if(mode == 1) {
		if(pNode->nodes[0] && pNode->nodes[2] && !pNode->nodes[1] && !pNode->nodes[3]) {
			pNode->nodes[0]->nodes[2] = pNode->nodes[2];
			pNode->nodes[2]->nodes[0] = pNode->nodes[0];
			outSet->insert(pNode);
			return;
		} else
		if(pNode->nodes[1] && pNode->nodes[3] && !pNode->nodes[0] && !pNode->nodes[2]) {
			pNode->nodes[1]->nodes[3] = pNode->nodes[3];
			pNode->nodes[3]->nodes[1] = pNode->nodes[1];
			outSet->insert(pNode);
			return;
		}

		int nodeConnectionCount = 0;
		for(int i = 0; i < 4; i++) {
			if(pNode->nodes[i] && outSet->find(pNode->nodes[i]) == outSet->end()) {
				nodeConnectionCount++;
			}
		}
		if(nodeConnectionCount >= 2) {
			return;
		}

		for(int i = 0; i < 4; i++) {
			if(pNode->nodes[i]) {
				pNode->nodes[i]->nodes[GetRevDirIndex(i)] = nullptr;
			}
		}
	}

	outSet->insert(pNode);

	for(int i = 0; i < 4; i++) {
		if(auto _pNodeIterating = pNode->nodes[i]; _pNodeIterating && outSet->find(_pNodeIterating) == outSet->end()) {
			PipeRecursion(outSet, _pNodeIterating, mode, pExtraDataForModes);
		}
	}

	if(mode == 2 && pNode->pDevice) {
		reinterpret_cast<std::vector<PipeNodeT *> *>(pExtraDataForModes)->push_back(pNode);
		const auto & pipeNodes = pNode->pDevice->GetPipeNodes();
		for(const auto & _pNodeIterating : pipeNodes) {
			if(_pNodeIterating != pNode && outSet->find(_pNodeIterating) == outSet->end()) {
				PipeRecursion(outSet, _pNodeIterating, mode, pExtraDataForModes);
			}
		}
	}
}