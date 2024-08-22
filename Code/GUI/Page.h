#pragma once

#include "IElement.h"

namespace GUI {

	class CPage
		: public IElement
		, public SContainerBase
	{
	public:
		CPage()
			: CPage { 0.f, 64.f }
		{}
		CPage(const Frame::Vec2 & leftTopRelative, const Frame::Vec2 & size)
			: m_ltRelative { leftTopRelative }
			, m_size { size }
			, m_pFramebuffer { std::make_shared<Frame::CFramebuffer>(Frame::Vec2Cast<int>(size)) }
		{}
		virtual ~CPage() = default;

		virtual Frame::Vec2 GetLeftTopRelative() const override {
			return m_ltRelative;
		}
		virtual Frame::Vec2 GetRightBottomRelative() const override {
			return m_ltRelative + m_size;
		}
		virtual void SetPosRelative(const Frame::Vec2 & pos) override {
			m_ltRelative = pos;
		}
		virtual void SetSize(const Frame::Vec2 & size) override {
			m_size = size;
		}

		virtual void SetPosAdd(const Frame::Vec2 & _posAdd) override;
		virtual void OnMouseOnMe(const Frame::Vec2 & mousePos) override;
		virtual void Step() override;
		virtual void Draw() override;

	protected:
		Frame::Vec2 m_ltRelative, m_size;
		std::shared_ptr<Frame::CFramebuffer> m_pFramebuffer;
	};

	class CDraggablePage : public CPage {
	public:
		CDraggablePage()
			: CPage {}
		{}
		CDraggablePage(const Frame::Vec2 & leftTopRelative, const Frame::Vec2 & size)
			: CPage { leftTopRelative, size }
		{}
		virtual ~CDraggablePage() = default;

		virtual bool PointInMe(const Frame::Vec2 & point) const override {
			return m_bDragging || CPage::PointInMe(point);
		}

		virtual void OnMouseOnMe(const Frame::Vec2 & mousePos) override;
		virtual void Step() override;

		bool IsDragging() const {
			return m_bDragging;
		}

	private:
		Frame::Vec2 m_relativeToMouse {};
		bool m_bDragging = false;
	};

	class CDraggableResizablePage : public CDraggablePage {
	public:
		CDraggableResizablePage() = delete;
		CDraggableResizablePage(const Frame::Vec2 & leftTopRelative, const Frame::Vec2i & cellSize, const Frame::Vec2i & gridSize, const Frame::Vec2i & gridMinSize)
			: CDraggablePage { leftTopRelative, 1.f }
			, m_gridMinSize { gridMinSize }
		{
			SetSize(cellSize, gridSize);
		}
		virtual ~CDraggableResizablePage() = default;

		virtual bool PointInMe(const Frame::Vec2 & point) const override {
			return m_bResizing || CDraggablePage::PointInMe(point);
		}

		virtual void OnMouseOnMe(const Frame::Vec2 & mousePos) override;
		virtual void Step() override;
		virtual void Draw() override;

		bool IsResizing() const {
			return m_bResizing;
		}

		virtual void SetSize(const Frame::Vec2 & size) override {
			const Frame::Vec2 gridSize = size / Frame::Vec2Cast(m_cellSize);
			SetGridSize({ static_cast<int>(std::round(gridSize.x)), static_cast<int>(std::round(gridSize.y)) });
		}
		void SetSize(const Frame::Vec2i & cellSize, const Frame::Vec2i & gridSize) {
			m_cellSize = cellSize;
			m_gridSize = gridSize;
			SynchElementSize();
		}

		void SetCellSize(const Frame::Vec2i & size) {
			m_cellSize = size;
			SynchElementSize();
		}
		void SetGridSize(const Frame::Vec2i & size) {
			m_gridSize = size;
			SynchElementSize();
		}
		void SetGridMinSize(const Frame::Vec2i & size) {
			m_gridMinSize = size;
			SynchElementSize();
		}

	private:
		void SynchElementSize() {
			m_gridSize.x = std::max(m_gridMinSize.x, m_gridSize.x);
			m_gridSize.y = std::max(m_gridMinSize.y, m_gridSize.y);
			CDraggablePage::SetSize(Frame::Vec2Cast(m_cellSize) * Frame::Vec2Cast(m_gridSize));
		}

		bool m_bResizing = false;
		Frame::Vec2i m_cellSize { 1 };
		Frame::Vec2i m_gridSize { 64 };
		Frame::Vec2i m_gridMinSize { 16 };
	};

}