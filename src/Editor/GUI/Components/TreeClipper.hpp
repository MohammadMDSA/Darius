#pragma once

#include <Utils/Common.hpp>
#include <imgui.h>

#ifndef D_GUI_COMPONENT
#define D_GUI_COMPONENT Darius::Editor::Gui::Components
#endif

namespace Darius::Editor::Gui::Components
{
	struct TreeViewClipper {
		// persist
		float cursor_end = 0;
		float cursor_visible_start = 0;
		UINT first_visible_index = 0;
		float last_scroll = 0;
		float cursor_visible_end = 0;
		UINT visible_end_index = 0;
		bool full_pass = true;

		// valid only between begin end
		bool scrolled;
		bool met_visible;
		bool last_is_visible;
		UINT idx;
		float y;
		bool finished;
		UINT count;

		// returns index of first visible top-level node
		UINT Begin(UINT _count) {
			count = _count;
			scrolled = ImGui::GetScrollY() != last_scroll;
			if (scrolled) full_pass = true;
			if (full_pass) Refresh();

			// skip invisible space
			ImGui::SetCursorPosY(cursor_visible_start);

			// init runtime data
			met_visible = false;
			last_is_visible = true;
			idx = first_visible_index;
			finished = idx >= count;

			return idx;
		}

		void Refresh() {
			full_pass = false;
			last_scroll = ImGui::GetScrollY();
			first_visible_index = 0;
			cursor_visible_start = 0;
			cursor_end = 0;
		}

		bool BeginNode() {
			y = ImGui::GetCursorPosY();
			return !finished;
		}

		void EndNode() {
			const bool visible = ImGui::IsItemVisible();
			const bool is_first_visible = visible && !met_visible;
			if (is_first_visible) {
				met_visible = true;
				first_visible_index = idx;
				cursor_visible_start = y;
			}
			if (met_visible && !visible) {
				last_is_visible = false;
				y = ImGui::GetCursorPosY();
				if (cursor_end != 0) {
					// something has expended or collapsed
					if (y != cursor_visible_end && cursor_visible_end != 0) full_pass = true;
					if (idx != visible_end_index && visible_end_index != 0) full_pass = true;
					finished = true;
					cursor_visible_end = y;
					visible_end_index = idx;
				}
			}
			++idx;
			if (idx == count) finished = true;
		}

		void End() {
			if (cursor_end == 0 || last_is_visible) {
				cursor_end = ImGui::GetCursorPosY();
			}
			else {
				ImGui::SetCursorPosY(cursor_end - 2); // TODO why -2
			}
		}
	};
}