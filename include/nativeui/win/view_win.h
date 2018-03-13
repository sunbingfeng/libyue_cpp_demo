// Copyright 2017 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_VIEW_WIN_H_
#define NATIVEUI_WIN_VIEW_WIN_H_

#include "nativeui/view.h"
#include "nativeui/gfx/win/painter_win.h"
#include "nativeui/win/window_win.h"

namespace nu {

class ScrollImpl;
class WindowImpl;

// Possible control types.
// We only add new types when it is necessary, otherwise a new View should
// usually just be |Container| or |View|.
enum class ControlType {
  View,
  Subwin,
  Scrollbar,
  // Button types.
  Button,
  Checkbox,
  Radio,
  // Container types, other types should NOT be appended after Container.
  Container,
  Scroll,
};

// The common base for native window based view and directui view.
class ViewImpl {
 public:
  virtual ~ViewImpl() {}

  /////////////////////////////////////////////////////////////////////////////
  // Core implementations, should be overriden for each kind of view

  // Change view's bounds, relative to window.
  virtual void SizeAllocate(const Rect& bounds);

  // Return the hittest result for pixel |point|.
  virtual UINT HitTest(const Point& point) const;

  // Set the parent view.
  virtual void SetParent(ViewImpl* parent);
  virtual void BecomeContentView(WindowImpl* parent);

  // Invalidate the |dirty| rect.
  virtual void Invalidate(const Rect& dirty);

  // Move focus to the view.
  virtual void SetFocus(bool focus);
  virtual bool HasFocus() const;

  // Show/Hide the view.
  virtual void SetVisible(bool visible);

  // Set styles.
  virtual void SetFont(Font* font);
  virtual void SetColor(Color color);
  virtual void SetBackgroundColor(Color color);

  /////////////////////////////////////////////////////////////////////////////
  // Events

  // Draw the content.
  virtual void Draw(PainterWin* painter, const Rect& dirty);

  // The DPI of this view has changed.
  virtual void OnDPIChanged() {}

  // The mouse events.
  virtual void OnMouseMove(NativeEvent event);
  virtual void OnMouseEnter(NativeEvent event);
  virtual void OnMouseLeave(NativeEvent event);
  virtual bool OnMouseWheel(bool vertical, UINT flags, int delta,
                            const Point& point) { return false; }
  virtual bool OnMouseClick(NativeEvent event);
  virtual bool OnKeyEvent(NativeEvent event);

  // Called when the view lost capture.
  virtual void OnCaptureLost();

  /////////////////////////////////////////////////////////////////////////////
  // Helpers

  // Get the mouse position in current view.
  Point GetMousePosition() const;

  // Get the size allocation that inside viewport.
  Rect GetClippedRect() const;

  // Invalidate the whole view.
  void Invalidate();

  // Set control's state.
  void SetState(ControlState state);
  ControlState state() const { return state_; }

  // Change the bounds without invalidating.
  void set_size_allocation(const Rect& bounds) { size_allocation_ = bounds; }
  Rect size_allocation() const { return size_allocation_; }

  // Whether the view is visible.
  bool is_visible() const { return is_visible_; }

  // Whether the view is disabled.
  bool is_disabled() const { return state_ == ControlState::Disabled; }

  // Whether the view can get focus.
  void set_focusable(bool focusable) { focusable_ = focusable; }
  bool is_focusable() const { return focusable_; }

  void set_draggable(bool draggable) { draggable_ = draggable; }
  bool is_draggable() const { return draggable_; }

  // Set control's viewport, only called by Scroll.
  void set_viewport(ScrollImpl* scroll) { viewport_ = scroll; }

  // Returns the DPI of current view.
  float scale_factor() const { return scale_factor_; }

  Font* font() const { return font_.get(); }
  Color color() const { return color_; }
  Color background_color() const { return background_color_; }

  WindowImpl* window() const { return window_; }
  ControlType type() const { return type_; }
  View* delegate() const { return delegate_; }

 protected:
  ViewImpl(ControlType type, View* delegate);

  // Called by SetParent/BecomeContentView when parent view changes.
  void ParentChanged();

 private:
  ControlType type_;

  // Whether the view can have focus.
  bool focusable_ = false;

  // Whether dragging the view can move the window.
  bool draggable_ = false;

  // Styles.
  scoped_refptr<Font> font_;
  Color color_;
  Color background_color_ = Color(0, 0, 0, 0);  // transparent

  // The focus state.
  bool is_focused_ = false;

  // The visible state.
  bool is_visible_ = true;

  // The control state.
  ControlState state_ = ControlState::Normal;

  // The window that owns the view.
  WindowImpl* window_ = nullptr;

  // The viewport that owns this view.
  ScrollImpl* viewport_ = nullptr;

  // The scale factor this view uses.
  float scale_factor_;

  // The view which this class implements, this can be nullptr;
  View* delegate_;

  // The absolute bounds relative to the origin of window.
  Rect size_allocation_;

  DISALLOW_COPY_AND_ASSIGN(ViewImpl);
};

}  // namespace nu

#endif  // NATIVEUI_WIN_VIEW_WIN_H_
