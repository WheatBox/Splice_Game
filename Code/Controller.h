#pragma once

#include <FrameMath/Vector2.h>
#include <FrameMath/ColorMath.h>

#include <unordered_set>
#include <tuple>
#include <memory>

namespace Controller {

	constexpr int gridCellSize = 16;
	constexpr int controllerMinWidth = 4;
	constexpr int controllerMinHeight = 4;

	enum class EElement {
		Unknown = 0,
		Button,
		END
	};

	struct IElement;

	struct SController {
		int gridWidth = 30; // 实际宽度 = gridWidth * gridCellSize
		int gridHeight = 20; // 实际高度 = gridHeight * gridCellSize
		
		std::unordered_set<std::shared_ptr<IElement>> elements;
	};

	struct IElement {
		IElement() = default;
		virtual ~IElement() = default;

		// return { { ltx, lty }, { rbx, rby } };
		virtual std::pair<Frame::Vec2i, Frame::Vec2i> GetAABB() const = 0;

		EElement element = EElement::Unknown;
		Frame::Vec2i pos {};
	};

	struct SButtonElement : public IElement {
		SButtonElement() {
			element = EElement::Button;
		}
		virtual ~SButtonElement() = default;
		static std::pair<Frame::Vec2i, Frame::Vec2i> GetAABBRelative() {
			return { { -2 }, { 2 } };
		}
		virtual std::pair<Frame::Vec2i, Frame::Vec2i> GetAABB() const override {
			auto [lt, rb] = GetAABBRelative();
			return { pos + lt, pos + rb };
		}
	};

	static inline Frame::Vec2 GetElementRealPos(const Frame::Vec2i & elementGridPos, const Frame::Vec2 & controllerLT) {
		return controllerLT + Frame::Vec2Cast(elementGridPos * Controller::gridCellSize);
	}
	static inline Frame::Vec2 GetElementRealPos(const std::shared_ptr<IElement> & pElement, const Frame::Vec2 & controllerLT) {
		return GetElementRealPos(pElement->pos, controllerLT);
	}

	static inline std::pair<Frame::Vec2, Frame::Vec2> GetElementAABBRealPos(const std::shared_ptr<IElement> & pElement, const Frame::Vec2 & controllerLT) {
		auto [lt, rb] = pElement->GetAABB();
		return { controllerLT + Frame::Vec2Cast(lt * Controller::gridCellSize), controllerLT + Frame::Vec2Cast(rb * Controller::gridCellSize) };
	}

	void DrawAABB(EElement element, const Frame::Vec2 & pos, Frame::ColorRGB rgb, float alpha);
	void DrawPreview(EElement element, const Frame::Vec2 & pos, float alpha, const Frame::Vec2 & scale);
	
	static inline bool AABBIntersect(const Frame::Vec2i & lt1, const Frame::Vec2i & rb1, const Frame::Vec2i & lt2, const Frame::Vec2i & rb2) {
		return rb1.x > lt2.x && lt1.x < rb2.x && rb1.y > lt2.y && lt1.y < rb2.y;
	}
	static inline bool AABBIntersect(const IElement & elem, const Frame::Vec2i & lt2, const Frame::Vec2i & rb2) {
		auto [lt1, rb1] = elem.GetAABB();
		return AABBIntersect(lt1, rb1, lt2, rb2);
	}
	static inline bool AABBIntersect(const IElement & elem1, const IElement & elem2) {
		auto [lt1, rb1] = elem1.GetAABB();
		auto [lt2, rb2] = elem2.GetAABB();
		return AABBIntersect(lt1, rb1, lt2, rb2);
	}
}