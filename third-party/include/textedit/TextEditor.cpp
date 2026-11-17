//	TextEditor - A syntax highlighting text editor for Dear ImGui.
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


//
//	Include files
//

#include <cstdint>
#include <cmath>
#include <set>

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "TextEditor.h"


//
//	TextEditor::TextEditor
//

TextEditor::TextEditor() {
	SetPalette(defaultPalette);
}


//
//	TextEditor::reset
//

void TextEditor::reset() {
	document.clear();
	transactions.reset();
	cursors.clearAll();
	clearMarkers();
	clearSquiggles();
	resetScrolling();
}


//
//	TextEditor::render
//

bool TextEditor::render(const char* title, const ImVec2& size, ImGuiChildFlags childFlags, ImGuiWindowFlags windowFlags) {
	// track content state changes
	bool documentChanged = false;

	// special handling for first frame as we can't use SetNextWindowFocus during it
	if (!firstFrame) {
		// ensure editor has focus (if required)
		if (focusOnEditor) {
			ImGui::SetNextWindowFocus();
			focusOnEditor = false;
		}

	} else {
		firstFrame = false;
	}

	// start a new child window
	ImGui::SetNextWindowContentSize(totalSize);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::ColorConvertU32ToFloat4(palette.get(Color::background)));

	if (ImGui::BeginChild(title, size, childFlags, windowFlags)) {
		// perform last part of scrolling reset here (as it requires the correct Dear ImGui context)
		if (resetDearImGuiScrolling) {
			ImGui::SetScrollX(0.0f);
			ImGui::SetScrollY(0.0f);
			resetDearImGuiScrolling = false;
		}

		// get drawList
		drawList = ImGui::GetWindowDrawList();

		// get font information
		font = ImGui::GetFont();
		fontSize = ImGui::GetFontSize();
		const auto& style = ImGui::GetStyle();
		fontScaleDpi = style.FontScaleDpi;
		glyphSize = ImVec2(ImGui::CalcTextSize("#").x, ImGui::GetTextLineHeightWithSpacing() * config.lineSpacing);

		// determine current position and visible size
		cursorScreenPos = ImGui::GetCursorScreenPos();
		visibleSize = ImGui::GetCurrentWindow()->InnerRect.GetSize();

		// determine horizontal offsets for folding indicator, line numbers, decorations and text
		foldIndicatorLeftOffset = config.leftMargin * glyphSize.x;
		foldIndicatorRightOffset = config.lineFolding ? foldIndicatorLeftOffset + glyphSize.x : foldIndicatorLeftOffset;

		lineNumberLeftOffset = foldIndicatorRightOffset;

		if (config.showLineNumbers) {
			const auto digits = static_cast<size_t>(std::log10(static_cast<float>(document.size() + 1)) + 1.0f);
			lineNumberRightOffset = lineNumberLeftOffset + digits * glyphSize.x;

		} else {
			lineNumberRightOffset = lineNumberLeftOffset;
		}

		if (decoratorWidth) {
			decorationOffset = lineNumberRightOffset + config.decorationMargin * glyphSize.x;
			textLeftOffset = decorationOffset + (decoratorWidth + config.textMargin) * glyphSize.x;

		} else {
			decorationOffset = lineNumberRightOffset;
			textLeftOffset = decorationOffset + config.textMargin * glyphSize.x;
		}

		if (config.showMiniMap) {
			miniMapRowHeight = 3.0f * fontScaleDpi;
			miniMapColumnHeight = 2.0f * fontScaleDpi;
			miniMapColumnWidth = 1.0f * fontScaleDpi;

			if (config.miniMapColumns == 0.0f) {
				miniMapWidth = std::floor(
					(visibleSize.x - textLeftOffset) /
					(glyphSize.x + miniMapColumnWidth)) * miniMapColumnWidth;

			} else {
				miniMapWidth = config.miniMapColumns * miniMapColumnWidth;
			}

			textRightOffset = visibleSize.x - miniMapWidth;
			miniMapOffset = textRightOffset;

		} else {
			textRightOffset = visibleSize.x;
		}

		// determine number of columns at which text will wrap
		textSize = ImVec2(textRightOffset - textLeftOffset, visibleSize.y);
		config.wordWrapColumns = static_cast<size_t>(std::max(std::floor(textSize.x / glyphSize.x), 0.0f));

		// handle possible state changes caused by API calls before first frame or between frames
		updateState();
		handlePossibleScrolling();

		// handle keyboard inputs
		handleKeyboardInputs();
		documentChanged = updateState();

		// handle mouse inputs
		handleMouseInteractions();

		// determine visible row/column limits
		firstVisibleRow = static_cast<size_t>(std::floor(ImGui::GetScrollY() / glyphSize.y));
		lastVisibleRow = std::min(static_cast<size_t>(std::ceil((ImGui::GetScrollY() + textSize.y) / glyphSize.y)), typeSetter.getRowCount() - 1);
		firstVisibleColumn = static_cast<size_t>(std::floor(ImGui::GetScrollX() / glyphSize.x));
		lastVisibleColumn = static_cast<size_t>(std::ceil((ImGui::GetScrollX() + textSize.x) / glyphSize.x));

		// update color palette (if required)
		if (paletteAlpha != style.Alpha) {
			updatePalettes();
		}

		// determine width of cursor
#if IMGUI_VERSION_NUM >= 19290
		cursorWidth =  style.InputTextCursorSize;
#else
		cursorWidth = 1.0f * fontScaleDpi;
#endif

		// setup clipping over the text area
		const auto offset = ImGui::GetWindowPos().x;

		drawList->PushClipRect(
			ImVec2(offset + textLeftOffset - cursorWidth, drawList->GetClipRectMin().y),
			ImVec2(offset + textRightOffset + cursorWidth, drawList->GetClipRectMax().y),
			true);

		// render parts in the text area
		renderCurrentLineHighlight();
		renderActiveBracketBackground();
		renderSelections();
		renderTextMarkers();
		renderMatchingBracketLines();
		renderSquiggles();
		renderText();
		renderCursorCarets();

		// end clipping
		drawList->PopClipRect();

		// render other parts
		renderLineNumberMarkers();
		renderLineNumbers();
		renderDecorations();
		renderFoldIndicators();
		renderMiniMap();
		renderScrollbarMiniMap();
		renderPanScrollIndicator();
		renderFindReplace();
		renderPopups();

		// handle scrolling caused by actions in this frame
		handlePossibleScrolling();

		// handle navigation cursor (if required)
		if (ImGui::GetIO().ConfigFlags & (ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad)) {
			// this is a series of hacks to get around Dear ImGui issues
			// the text editor is a custom widget build on a ChildWindow
			// this works great as a ChildWindow provides all required functionality (like scrolling)
			// ChildWindow does however not work well as a base class when navigation cursors are used
			// ChildWindow has flags to avoid navigation cursors (e.g. ImGuiWindowFlags_NoNavInputs)
			// ChildWindow does not have a flag to force navigation inputs (hence the need for these hacks)
			auto window = ImGui::GetCurrentWindow();

			if (ImGui::IsWindowFocused()) {
				// ImGui::BeginChildEx: "we can enter a child if (A) it has navigable items or (B) it can be scrolled"
				// as a result this editor will not get a navigation border if it doesn't have a vertical scrollbar
				// so we need to draw it ourselves in that case
				if (ImGui::GetCurrentContext()->NavCursorVisible && !window->ScrollbarY) {
					ImRect bb{ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize()};
					bb.Expand(ImVec2(2.0f, 2.0f));

					window->ParentWindow->DrawList->AddRect(
						bb.Min,
						bb.Max,
						ImGui::GetColorU32(ImGuiCol_NavCursor),
						ImGui::GetStyle().FrameRounding,
						fontScaleDpi);
				}

			} else {
				// this voodoo is required to get a navigation cursor when the editor is not focussed
				// ImGui::InputTextEx: "this is to ensure that EndChild() will display a navigation highlight so we can "enter" into it"
				window->DC.NavLayersActiveMaskNext |= (1 << window->DC.NavLayerCurrent);
			}
		}
	}

	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();

	// handle change tracking if there is a callback in place
	if (delayedChangeCallback && delayedChangeDetected) {
		if (std::chrono::system_clock::now() > delayedChangeReportTime) {
			delayedChangeCallback();
			delayedChangeDetected = false;
		}
	}

	return documentChanged;
}


//
//	TextEditor::renderCurrentLineHighlight
//

void TextEditor::renderCurrentLineHighlight() {
	if (config.showCurrentLineHighlight && !cursors.anyHasSelection()) {
		std::set<size_t> alreadyDrawn{};

		for (const auto& cursor : cursors) {
			const auto lineNumber = cursor.getInteractiveEnd().line;
			const auto [iterator, inserted] = alreadyDrawn.insert(lineNumber);

			if (inserted) {
				const auto& line = document[lineNumber];
				const auto topLeft = ImVec2(textLeftOffset, cursorScreenPos.y + line.row * glyphSize.y);
				const auto bottomRight = topLeft + ImVec2(textRightOffset, line.rows * glyphSize.y);

				drawList->AddRectFilled(topLeft, bottomRight, palette.get(Color::currentLineHighlight));
				drawList->AddRect(topLeft, bottomRight, palette.get(Color::currentLineHighlightBorder));
			}
		}
	}
}


//
//	TextEditor::renderActiveBracketBackground
//

void TextEditor::renderActiveBracketBackground() {
	if (config.showMatchingBrackets && bracketeer.size()) {
		// render active bracket pair
		const auto active = bracketeer.getEnclosingBrackets(cursors.getMain().getInteractiveEnd());

		if (active != bracketeer.end() && active->visible) {
			if (document[active->start.line].foldingState != FoldingState::hidden) {
				const auto startVis = docPos2VisPos(active->start);

				if (startVis.row >= firstVisibleRow && startVis.row <= lastVisibleRow) {
					const auto x = cursorScreenPos.x + textLeftOffset + startVis.column * glyphSize.x;
					const auto y = cursorScreenPos.y + startVis.row * glyphSize.y;

					drawList->AddRectFilled(
						ImVec2(x, y),
						ImVec2(x + glyphSize.x, y + glyphSize.y),
						palette.get(Color::matchingBracketBackground));
				}
			}

			if (document[active->end.line].foldingState != FoldingState::hidden) {
				const auto endVis = docPos2VisPos(active->end);

				if (endVis.row >= firstVisibleRow && endVis.row <= lastVisibleRow) {
					const auto x = cursorScreenPos.x + textLeftOffset + endVis.column * glyphSize.x;
					const auto y = cursorScreenPos.y + endVis.row * glyphSize.y;

					drawList->AddRectFilled(
						ImVec2(x, y),
						ImVec2(x + glyphSize.x, y + glyphSize.y),
						palette.get(Color::matchingBracketBackground));
				}
			}
		}
	}
}


//
//	TextEditor::renderSelections
//

void TextEditor::renderSelections() {
	// draw background for selections
	for (const auto& cursor : cursors) {
		if (cursor.hasSelection()) {
			const auto begin = cursor.getSelectionStart();
			const auto end = cursor.getSelectionEnd();

			for (size_t i = begin.line; i <= end.line; i++) {
				const auto& line = document[i];

				if (line.foldingState != FoldingState::hidden) {
					if (line.rows == 1) {
						if (line.row >= firstVisibleRow && line.row <= lastVisibleRow) {
							const auto lineLeft = DocPos(i, 0);
							const auto lineRight =DocPos(i, line.size());
							const auto docLeft = begin <= lineLeft ? lineLeft : begin;
							const auto docRight = end > lineRight ? lineRight : end;
							const auto visLeft = docPos2VisPos(docLeft);
							const auto visRight = docPos2VisPos(docRight);

							const auto x = cursorScreenPos.x + textLeftOffset;
							const auto left = x + visLeft.column * glyphSize.x;
							const auto right = x + visRight.column * glyphSize.x;
							const auto y = cursorScreenPos.y + line.row * glyphSize.y;

							drawList->AddRectFilled(
								ImVec2(left, y),
								ImVec2(right, y + glyphSize.y),
								palette.get(Color::selection));
							}

					} else {
						for (size_t j = 0; j < line.rows; j++) {
							const auto& section = line.sections->at(j);
							const auto row = line.row + j;

							if (row >= firstVisibleRow && row <= lastVisibleRow) {
								const auto sectionLeft = DocPos(i, section.startIndex);
								const auto sectionRight = DocPos(i, section.endIndex);

								if (begin < sectionRight && end > sectionLeft) {
									const auto visLeft = begin <= sectionLeft ? VisPos(row, 0) : docPos2VisPos(begin);
									const auto visRight = end > sectionRight ? VisPos(row, section.columns) : docPos2VisPos(end);

									const auto x = cursorScreenPos.x + textLeftOffset;
									const auto left = x + visLeft.column * glyphSize.x;
									const auto right = x + visRight.column * glyphSize.x;
									const auto y = cursorScreenPos.y + row * glyphSize.y;

									drawList->AddRectFilled(
										ImVec2(left, y),
										ImVec2(right, y + glyphSize.y),
										palette.get(Color::selection));
								}
							}
						}
					}
				}
			}
		}
	}
}


//
//	TextEditor::renderTextMarkers
//

void TextEditor::renderTextMarkers() {
	if (markers.size()) {
		for (size_t row = firstVisibleRow; row <= lastVisibleRow; row++) {
			const auto markerIndex = document[typeSetter[row].line].marker;

			if (markerIndex) {
				const auto& marker = markers[markerIndex - 1];
				const auto y = cursorScreenPos.y + row * glyphSize.y;

				if (((marker.textColor >> IM_COL32_A_SHIFT) & 0xFF) != 0) {
					const auto left = cursorScreenPos.x + textLeftOffset;
					const auto right = left + lastVisibleColumn * glyphSize.x;
					const auto start = ImVec2(left, y);
					const auto end = ImVec2(right, y + glyphSize.y);
					drawList->AddRectFilled(start, end, marker.textColor);

					if (marker.textTooltip.size() && ImGui::IsMouseHoveringRect(start, end)) {
						if (ImGui::BeginTooltip()) {
							ImGui::PushStyleColor(ImGuiCol_PopupBg, marker.textColor);
							ImGui::TextUnformatted(marker.textTooltip.c_str());
							ImGui::PopStyleColor();
							ImGui::EndTooltip();
						}
					}
				}
			}
		}
	}
}


//
//	TextEditor::renderMatchingBracketLines
//

void TextEditor::renderMatchingBracketLines() {
	if (config.showMatchingBrackets && bracketeer.size()) {
		const auto active = bracketeer.getEnclosingBrackets(cursors.getMain().getInteractiveEnd());

		// render bracket pair lines
		for (auto bracket = bracketeer.begin(); bracket < bracketeer.end(); bracket++) {
			if (bracket->visible &&
				document[bracket->start.line].foldingState != FoldingState::folded &&
				document[bracket->start.line].foldingState != FoldingState::hidden) {

				const auto startVisPos = docPos2VisPos(bracket->start);
				const auto endVisPos = docPos2VisPos(bracket->end);

				if (startVisPos.row <= lastVisibleRow &&
					endVisPos.row >= firstVisibleRow &&
					endVisPos.row - startVisPos.row > 1) {

					const auto column = std::min(startVisPos.column, endVisPos.column);
					const auto lineX = cursorScreenPos.x + textLeftOffset + column * glyphSize.x;
					const auto startY = cursorScreenPos.y + std::max(startVisPos.row + 1, firstVisibleRow) * glyphSize.y;
					const auto endY = cursorScreenPos.y + std::min(endVisPos.row, lastVisibleRow) * glyphSize.y;

					const auto color = palette.get(bracket == active ? Color::matchingBracketActive : Color::whitespace);
					drawList->AddLine(ImVec2(lineX, startY), ImVec2(lineX, endY), color);
				}
			}
		}
	}
}

//
//	renderSquiggle
//

static inline void renderSquiggle(ImDrawList* drawList, float left, float right, float top, float bottom, float thickness, ImU32 color, const char* tooltip, TextEditor::SquiggleStyle style) {
	const ImVec2 topLeft{left, top};
	const ImVec2 bottomRight{right, bottom};

	switch (style) {
		case TextEditor::SquiggleStyle::wave: {
			const auto height = bottom - top;
			const auto size = height * 0.2f;
			const auto offset = top + height * 0.8f;

			ImVec2 point(left, offset);
			bool down = true;

			drawList->PushClipRect(topLeft, bottomRight, true);

			while (point.x < right) {
				const ImVec2 next{point.x + size, down ? offset + size : offset};
				drawList->AddLine(point, next, color, thickness);
				point = next;
				down = !down;
			}

			drawList->PopClipRect();
			break;
		}

		case TextEditor::SquiggleStyle::background:
			drawList->AddRectFilled(topLeft, bottomRight, color);
			break;
	}

	if (*tooltip && ImGui::IsMouseHoveringRect(topLeft, bottomRight)) {
		if (ImGui::BeginTooltip()) {
			ImGui::TextUnformatted(tooltip);
			ImGui::EndTooltip();
		}
	}
}


//
//	TextEditor::renderSquiggles
//

void TextEditor::renderSquiggles() {
	if (squiggles.size()) {
		ImVec2 rowScreenPos = cursorScreenPos + ImVec2(textLeftOffset, firstVisibleRow * glyphSize.y);

		// only process all visible rows
		for (size_t i = firstVisibleRow; i <= lastVisibleRow; i++) {
			// determine visible boundaries for this row
			const auto& line = document[typeSetter[i].line];
			size_t index;
			size_t column;
			size_t endColumn;

			if (config.wordWrap && line.sections) {
				const auto& section = line.sections->at(typeSetter[i].section);
				index = section.startIndex;
				column = section.indent;
				endColumn = section.columns;

			} else {
				index = 0;
				column = 0;
				endColumn = line.columns;
			}

			// setup squiggles
			bool inSquiggle = false;
			size_t squiggleIndex = 0;
			float squiggleLeft = 0.0f;
			const float thickness = 1.2f * fontScaleDpi;

			// only process all visible columns
			while (column < endColumn && column <= lastVisibleColumn) {
				const auto& glyph = line[index++];
				const ImVec2 glyphPos(rowScreenPos.x + column * glyphSize.x, rowScreenPos.y);

				// handle squiggles
				if (glyph.squiggle) {
					const auto nextIndex = glyph.squiggle - 1;

					if (inSquiggle) {
						if (squiggleIndex != nextIndex) {
							// render squiggle and start new one
							const auto& squiggle = squiggles[squiggleIndex];
							renderSquiggle(drawList, squiggleLeft, glyphPos.x, glyphPos.y, glyphPos.y + glyphSize.y, thickness, squiggle.color, squiggle.tooltip.c_str(), squiggle.style);
							squiggleIndex = nextIndex;
							squiggleLeft = glyphPos.x;
						}

					} else {
						// start new squiggle
						inSquiggle = true;
						squiggleIndex = nextIndex;
						squiggleLeft = glyphPos.x;
					}

				} else if (inSquiggle) {
					// render squiggle
					const auto& squiggle = squiggles[squiggleIndex];
					renderSquiggle(drawList, squiggleLeft, glyphPos.x, glyphPos.y, glyphPos.y + glyphSize.y, thickness, squiggle.color, squiggle.tooltip.c_str(), squiggle.style);
					inSquiggle = false;
				}

				column += glyph.columns;
			}

			if (inSquiggle) {
				// render last squiggle on line
				const auto& squiggle = squiggles[squiggleIndex];
				const auto glyphPos = cursorScreenPos + ImVec2(textLeftOffset + typeSetter[i].columns * glyphSize.x, i * glyphSize.y);
				renderSquiggle(drawList, squiggleLeft, glyphPos.x, glyphPos.y, glyphPos.y + glyphSize.y, thickness, squiggle.color, squiggle.tooltip.c_str(), squiggle.style);
			}

			rowScreenPos.y += glyphSize.y;
		}
	}
}


//
//	TextEditor::renderText
//

void TextEditor::renderText() {
	ImVec2 rowScreenPos = cursorScreenPos + ImVec2(textLeftOffset, firstVisibleRow * glyphSize.y);
	const auto firstRenderableColumn = (firstVisibleColumn / config.tabSize) * config.tabSize;

	// only process all visible rows
	for (size_t i = firstVisibleRow; i <= lastVisibleRow; i++) {
		// determine visible boundaries for this row
		const auto& line = document[typeSetter[i].line];
		size_t index;
		size_t column;
		size_t endColumn;

		if (config.wordWrap && line.sections) {
			const auto& section = line.sections->at(typeSetter[i].section);
			index = section.startIndex;
			column = section.indent;
			endColumn = section.columns;

		} else {
			index = 0;
			column = 0;
			endColumn = line.columns;
		}

		// only process all visible columns
		while (column < endColumn && column <= lastVisibleColumn) {
			const auto& glyph = line[index++];
			const auto codepoint = glyph.codepoint;
			const ImVec2 glyphPos(rowScreenPos.x + column * glyphSize.x, rowScreenPos.y);

			// handle tabs
			if (codepoint == '\t') {
				if (config.showTabs && column >= firstRenderableColumn) {
					const auto x1 = glyphPos.x + glyphSize.x * 0.3f;
					const auto x2 = glyphPos.x + (config.showShortTabs ? glyphSize.x : glyphSize.x * static_cast<float>(glyph.columns) - 1.0f);
					const auto y = glyphPos.y + fontSize * 0.5f;

					ImVec2 p1, p2, p3, p4;
					p1 = ImVec2(x1, y);
					p2 = ImVec2(x2, y);
					p3 = ImVec2(x2 - fontSize * 0.16f, y - fontSize * 0.16f);
					p4 = ImVec2(x2 - fontSize * 0.16f, y + fontSize * 0.16f);

					drawList->AddLine(p1, p2, palette.get(Color::whitespace));
					drawList->AddLine(p2, p3, palette.get(Color::whitespace));
					drawList->AddLine(p2, p4, palette.get(Color::whitespace));
				}

			// handle spaces
			} else if (codepoint == ' ') {
				if (config.showSpaces && column >= firstRenderableColumn) {
					const auto x = glyphPos.x + glyphSize.x * 0.5f;
					const auto y = glyphPos.y + fontSize * 0.5f;
					drawList->AddCircleFilled(ImVec2(x, y), 1.5f, palette.get(Color::whitespace), 4);
				}

			// handle regular glyphs
			} else {
				if (column >= firstRenderableColumn) {
					font->RenderChar(drawList, fontSize, glyphPos, palette.get(glyph.color), codepoint);
				}
			}

			column += glyph.columns;
		}

		// draw ellipsis at the end of folded lines
		if (i == line.row + line.rows - 1) {
			if (line.foldingState == FoldingState::folded) {
				const auto glyphPos = cursorScreenPos + ImVec2(textLeftOffset + typeSetter[i].columns * glyphSize.x, i * glyphSize.y);
				font->RenderChar(drawList, fontSize, glyphPos, palette.get(Color::text), font->EllipsisChar);
			}
		}

		rowScreenPos.y += glyphSize.y;
	}
}


//
//	TextEditor::renderCursorCarets
//

void TextEditor::renderCursorCarets() {
	if (config.caretsVisible && ImGui::IsWindowFocused()) {
		const auto& io = ImGui::GetIO();
		cursorAnimationTimer += io.DeltaTime;
		size_t cursorIndex = 0;

		for (const auto& cursor : cursors) {
			const auto docPos = cursor.getInteractiveEnd();

			if (document[docPos.line].foldingState != FoldingState::hidden) {
				const auto pos = docPos2VisPos(docPos);

				if (pos.row >= firstVisibleRow && pos.row <= lastVisibleRow && pos.column >= firstVisibleColumn && pos.column <= lastVisibleColumn) {
					const auto caretVisible = !io.ConfigInputTextCursorBlink || cursorAnimationTimer <= 0.0f || std::fmod(cursorAnimationTimer, 1.2f) <= 0.8f;
					const auto x = cursorScreenPos.x + textLeftOffset + pos.column * glyphSize.x;
					const auto y = cursorScreenPos.y + pos.row * glyphSize.y;

					// handle custom caret renderer
					if (customCaretCallback) {
						CustomCaret caret;
						caret.drawList = drawList;
						caret.glyphPos = ImVec2(x, y);
						caret.glyphSize = glyphSize;
						caret.caretVisible = caretVisible;
						caret.caretColor = palette.get(Color::cursor);
						caret.cursorIndex = cursorIndex;
						customCaretCallback(caret);

					} else if (caretVisible) {
						drawList->AddRectFilled(
							ImVec2(x - cursorWidth, y),
							ImVec2(x, y + glyphSize.y),
							palette.get(Color::cursor));
					}
				}
			}

			cursorIndex++;
		}
	}
}


//
//	TextEditor::renderLineNumberMarkers
//

void TextEditor::renderLineNumberMarkers() {
	if (markers.size()) {
		for (size_t row = firstVisibleRow; row <= lastVisibleRow; row++) {
			const auto markerIndex = document[typeSetter[row].line].marker;

			if (markerIndex) {
				const auto& marker = markers[markerIndex - 1];
				const auto y = cursorScreenPos.y + row * glyphSize.y;

				if (((marker.lineNumberColor >> IM_COL32_A_SHIFT) & 0xFF) != 0) {
					const auto left = cursorScreenPos.x + lineNumberLeftOffset;
					const auto right = cursorScreenPos.x + lineNumberRightOffset;
					const auto start = ImVec2(left, y);
					const auto end = ImVec2(right, y + glyphSize.y);
					drawList->AddRectFilled(start, end, marker.lineNumberColor);

					if (marker.lineNumberTooltip.size() && ImGui::IsMouseHoveringRect(start, end)) {
						if (ImGui::BeginTooltip()) {
							ImGui::PushStyleColor(ImGuiCol_PopupBg, marker.lineNumberColor);
							ImGui::TextUnformatted(marker.lineNumberTooltip.c_str());
							ImGui::PopStyleColor();
							ImGui::EndTooltip();
						}
					}
				}
			}
		}
	}
}


//
//	TextEditor::renderLineNumbers
//

void TextEditor::renderLineNumbers() {
	if (config.showLineNumbers) {
		if (customLineNumberCallback) {
			const auto position = ImVec2(ImGui::GetWindowPos().x + lineNumberLeftOffset, cursorScreenPos.y);
			const auto curserLine = cursors.getCurrent().getInteractiveEnd().line;

			CustomLineNumber data;
			data.drawList = ImGui::GetWindowDrawList();
			const auto width = lineNumberRightOffset - lineNumberLeftOffset;
			data.size = data.pos + ImVec2(width, glyphSize.y);
			data.digits = static_cast<size_t>(width / glyphSize.x);
			data.cursorLineNumber = cursors.getCurrent().getInteractiveEnd().line;

			for (size_t i = firstVisibleRow; i <= lastVisibleRow; i++) {
				if (typeSetter[i].section == 0) {
					data.pos = position + ImVec2(0.0f, i * glyphSize.y);
					data.lineNumber = typeSetter[i].line;
					const auto foreground = (data.lineNumber == curserLine) ? Color::currentLineNumber : Color::lineNumber;
					data.color = palette.get(foreground);
					customLineNumberCallback(data);
				}
			}

		} else {
			const auto curserLine = cursors.getCurrent().getInteractiveEnd().line;
			const auto position = ImVec2(ImGui::GetWindowPos().x + lineNumberRightOffset, cursorScreenPos.y);

			for (size_t i = firstVisibleRow; i <= lastVisibleRow; i++) {
				if (typeSetter[i].section == 0) {
					const auto lineNo = typeSetter[i].line + 1;
					const auto width = static_cast<size_t>(std::log10(lineNo) + 1.0f) * glyphSize.x;
					const auto foreground = (typeSetter[i].line == curserLine) ? Color::currentLineNumber : Color::lineNumber;
					const auto number = std::to_string(lineNo);
					drawList->AddText(position + ImVec2(-width, i * glyphSize.y), palette.get(foreground), number.c_str());
				}
			}
		}
	}
}


//
//	TextEditor::renderDecorations
//

void TextEditor::renderDecorations() {
	if (decoratorWidth && decoratorCallback) {
		auto position = ImVec2(ImGui::GetWindowPos().x + decorationOffset, cursorScreenPos.y + glyphSize.y * firstVisibleRow);
		const auto widthInPixels = decoratorWidth * glyphSize.x;
		Decorator decorator{0, widthInPixels, glyphSize.y, glyphSize, nullptr};

		for (size_t i = firstVisibleRow; i <= lastVisibleRow; i++) {
			if (typeSetter[i].section == 0) {
				decorator.line = typeSetter[i].line;
				decorator.userData = document.getUserData(i);
				ImGui::SetCursorScreenPos(position);
				ImGui::PushID(static_cast<int>(i));
				decoratorCallback(decorator);
				ImGui::PopID();
				position.y += glyphSize.y;
			}
		}

		ImGui::SetCursorScreenPos(cursorScreenPos);
	}
}


//
//	TextEditor::renderFoldIndicators
//

void TextEditor::renderFoldIndicators() {
	if (config.lineFolding) {
		const auto color = palette.get(Color::lineNumber);
		const auto hoveredColor = palette.get(Color::currentLineNumber);
		const auto size = fontSize * 0.25f;

		for (size_t i = firstVisibleRow; i <= lastVisibleRow; i++) {
			const auto& line = document[typeSetter[i].line];
			const auto foldingState = line.foldingState;

			if (typeSetter[i].section == 0) {
				if (foldingState == FoldingState::foldable || foldingState == FoldingState::folded) {
					const auto center = ImVec2(
						ImGui::GetWindowPos().x + foldIndicatorLeftOffset,
						cursorScreenPos.y + glyphSize.y * i + fontSize / 2.0f);

					const auto topLeft = center - ImVec2(glyphSize.x / 2.0f, fontSize / 2.0f);
					const auto bottomRight = center + ImVec2(glyphSize.x / 2.0f, fontSize / 2.0f);
					const auto isHovered = ImGui::IsMouseHoveringRect(topLeft, bottomRight);

					if (foldingState == FoldingState::foldable) {
						drawList->AddLine(
							ImVec2(center.x - size, center.y - size * 0.5f),
							ImVec2(center.x, center.y + size),
							isHovered ? hoveredColor : color);

						drawList->AddLine(
							ImVec2(center.x, center.y + size),
							ImVec2(center.x + size, center.y - size * 0.5f),
							isHovered ? hoveredColor : color);

					} else {
						drawList->AddLine(
							ImVec2(center.x - size * 0.5f, center.y - size),
							ImVec2(center.x + size, center.y),
							isHovered ? hoveredColor : color);

						drawList->AddLine(
							ImVec2(center.x + size, center.y),
							ImVec2(center.x - size * 0.5f, center.y + size),
							isHovered ? hoveredColor : color);
					}

					if (isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
						lineFold.toggleAtLine(document, typeSetter[i].line);
					}
				}
			}
		}

		ImGui::SetCursorScreenPos(cursorScreenPos);
	}
}


//
//	void TextEditor::renderMiniMap
//

void TextEditor::renderMiniMap() {
	if (config.showMiniMap) {
		// reset background colors
		for (auto& row : miniMap.rows) {
			row.color = 0;
		}

		// color cursor lines
		for (const auto& cursor : cursors) {
			const auto begin = cursor.getSelectionStart();
			const auto end = cursor.getSelectionEnd();

			for (size_t i = begin.line; i <= end.line; i++) {
				const auto& line = document[i];

				if (line.foldingState != FoldingState::hidden) {
					for (size_t j = 0; j < line.rows; j++) {
						miniMap.rows[line.row + j].color = miniMapPalette.get(Color::selection);
					}
				}
			}
		}

		// color marker lines
		if (markers.size()) {
			for (size_t row = 0; row < typeSetter.getRowCount(); row++) {
				const auto& line = document[typeSetter[row].line];

				if (line.marker) {
					auto color = markers[line.marker - 1].textColor;

					if (!color) {
						color = markers[line.marker - 1].lineNumberColor;
					}

					auto fColor = ImGui::ColorConvertU32ToFloat4(color);
					fColor.w *= miniMapAlpha;
					miniMap.rows[row].color = ImGui::ColorConvertFloat4ToU32(fColor);
				}
			}
		}

		// determine viewport information
		const auto totalMiniMapRows = miniMap.rows.size();
		const auto visibleMiniMapRows = (textSize.y / miniMapRowHeight);

		if (totalMiniMapRows < visibleMiniMapRows) {
			firstMiniMapRow = 0;
			lastMiniMapRow = miniMap.rows.size();

		} else {
			const auto scrollRatio = ImGui::GetScrollY() / ImGui::GetScrollMaxY();
			firstMiniMapRow = static_cast<size_t>(scrollRatio * (totalMiniMapRows - visibleMiniMapRows));
			lastMiniMapRow = firstMiniMapRow + static_cast<size_t>(std::ceil(visibleMiniMapRows));
			lastMiniMapRow = std::min(lastMiniMapRow, totalMiniMapRows);
		}

		// process all visible minimap rows
		auto pos = ImGui::GetWindowPos() + ImVec2(miniMapOffset, 0.0f);

		for (size_t i = firstMiniMapRow; i < lastMiniMapRow; i++) {
			const auto& row = miniMap.rows[i];

			// render line background
			if (row.color) {
				drawList->AddRectFilled(
					pos,
					pos + ImVec2(miniMapWidth, miniMapRowHeight),
					row.color);
			}

			// render text sections
			for (const auto& section : row.sections) {
				drawList->AddRectFilled(
					pos + ImVec2(section.start * miniMapColumnWidth, 0.0f),
					pos + ImVec2(section.end * miniMapColumnWidth, miniMapColumnHeight),
					miniMapPalette.get(section.color));
			}

			pos += ImVec2(0.0f, miniMapRowHeight);
		}

		// render viewport
		if (totalSize.y > textSize.y) {
			const auto viewPortStart = (firstVisibleRow - firstMiniMapRow) * miniMapRowHeight;
			const auto viewportHeight = (lastVisibleRow - firstVisibleRow) * miniMapRowHeight;
			const auto ViewPortTopLeft = ImGui::GetWindowPos() + ImVec2(miniMapOffset, viewPortStart);
			const auto viewPortBottomRight = ViewPortTopLeft + ImVec2(miniMapWidth, viewportHeight);

			auto fColor = ImGui::ColorConvertU32ToFloat4(palette.get(Color::text));
			fColor.w *= miniMapIsScrollbar ? miniMapViewPortActiveAlpha : miniMapViewPortAlpha;
			drawList->AddRectFilled(ViewPortTopLeft, viewPortBottomRight, ImGui::ColorConvertFloat4ToU32(fColor));
		}
	}
}


//
//	TextEditor::renderScrollbarMiniMap
//

void TextEditor::renderScrollbarMiniMap() {
	if (config.showScrollbarMiniMap) {
		// based on https://github.com/ocornut/imgui/issues/3114
		const auto window = ImGui::GetCurrentWindow();

		if (window->ScrollbarY) {
			const auto rect = ImGui::GetWindowScrollbarRect(window, ImGuiAxis_Y);
			const auto rowHeight = rect.GetHeight() / static_cast<float>(typeSetter.getRowCount());
			const auto offset = (rect.Max.x - rect.Min.x) * 0.3f;
			const auto left = rect.Min.x + offset;
			const auto right = rect.Max.x - offset;

			drawList->PushClipRect(rect.Min, rect.Max, false);

			// render cursor locations
			for (const auto& cursor : cursors) {
				const auto begin = cursor.getSelectionStart();
				const auto end = cursor.getSelectionEnd();

				for (size_t i = begin.line; i <= end.line; i++) {
					const auto& line = document[i];

					if (line.foldingState != FoldingState::hidden) {
						const auto ly1 = std::round(rect.Min.y + line.row * rowHeight);
						const auto ly2 = std::round(rect.Min.y + (line.row + line.rows) * rowHeight);

						drawList->AddRectFilled(
							ImVec2(left, ly1),
							ImVec2(right, ly2),
							palette.get(Color::selection));
					}
				}
			}

			// render marker locations
			if (markers.size()) {
				for (size_t row = 0; row < typeSetter.getRowCount(); row++) {
					const auto& line = document[typeSetter[row].line];

					if (line.marker) {
						auto color = markers[line.marker - 1].textColor;

						if (!color) {
							color = markers[line.marker - 1].lineNumberColor;
						}

						const auto ly = std::round(rect.Min.y + row * rowHeight);
						drawList->AddRectFilled(ImVec2(left, ly), ImVec2(right, ly + rowHeight), color);
					}
				}
			}

			drawList->PopClipRect();
		}
	}
}


//
//	TextEditor::renderPanScrollIndicator
//

void TextEditor::renderPanScrollIndicator() {
	if (config.showPanScrollIndicator && (panning || scrolling)) {
		const auto center = ImGui::GetWindowPos() + ImGui::GetWindowSize() / 2.0f;
		static constexpr int alpha = 160;
		drawList->AddCircleFilled(center, 20.0f, IM_COL32(255, 255, 255, alpha));
		drawList->AddCircle(center, 5.0f, IM_COL32(0, 0, 0, alpha), 0, 2.0f);

		drawList->AddTriangle(
			ImVec2(center.x - 15.0f, center.y),
			ImVec2(center.x - 8.0f, center.y - 4.0f),
			ImVec2(center.x - 8.0f, center.y + 4.0f),
			IM_COL32(0, 0, 0, alpha),
			2.0f);

		drawList->AddTriangle(
			ImVec2(center.x + 15.0f, center.y),
			ImVec2(center.x + 8.0f, center.y - 4.0f),
			ImVec2(center.x + 8.0f, center.y + 4.0f),
			IM_COL32(0, 0, 0, alpha),
			2.0f);

		drawList->AddTriangle(
			ImVec2(center.x, center.y - 15.0f),
			ImVec2(center.x - 4.0f, center.y - 8.0f),
			ImVec2(center.x + 4.0f, center.y - 8.0f),
			IM_COL32(0, 0, 0, alpha),
			2.0f);

		drawList->AddTriangle(
			ImVec2(center.x, center.y + 15.0f),
			ImVec2(center.x - 4.0f, center.y + 8.0f),
			ImVec2(center.x + 4.0f, center.y + 8.0f),
			IM_COL32(0, 0, 0, alpha),
			2.0f);
	}
}


//
//	TextEditor::renderPopups
//

void TextEditor::renderPopups() {
	PopupData popupData;
	popupData.pos = popupDocPos;
	popupData.userData = document.getUserData(popupDocPos.line);

	if (ImGui::BeginPopup("LineNumberContextMenu")) {
		if (lineNumberContextMenuCallback) {
			lineNumberContextMenuCallback(popupData);

		} else {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (ImGui::BeginPopup("TextContextMenu")) {
		if (textContextMenuCallback) {
			textContextMenuCallback(popupData);

		} else {
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	if (ImGui::IsPopupOpen("TextHoverPopup")) {
		ImGui::SetNextWindowPos(popupWindowPos, ImGuiCond_Always, ImVec2(0.0f, 1.0f));

		if (ImGui::BeginPopup("TextHoverPopup", ImGuiWindowFlags_NoFocusOnAppearing)) {
			if (textHoverCallback) {
				ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
				textHoverCallback(popupData);

			} else {
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	// render autocomplete popup
	if (autocomplete.render(document, cursors, typeSetter, config.language, textLeftOffset, glyphSize)) {
		// user picked a suggestion so insert it
		const auto start = autocomplete.getReplacementStart();
		const auto end = autocomplete.getReplacementEnd(document);
		const auto replacement = autocomplete.getReplacement();
		replaceSectionText(start, end, replacement);
	}
}


//
//	TextEditor::updateState
//

bool TextEditor::updateState() {
	// this function gets called to handle possible changes caused by the API or user interactions
	// the overlays determine what they need to do to update their state (could be nothing)
	colorizer.update(config, document);
	bracketeer.update(config, document);
	lineFold.update(config, document, bracketeer);
	cursors.update(document);

	float unused;
	const auto firstVisibleLineFraction = std::modf(ImGui::GetScrollY() / glyphSize.y, &unused);
	const auto previousFirstLine = visPos2DocPos(VisPos(firstVisibleRow, 0)).line;

	if (typeSetter.update(config, document, lineFold)) {
		// see if we can scroll to preserve the first visible line
		// but we don't overrule outstanding requests
		if (scrollToLineNumber == invalidLine && scrollToDocPos.line == invalidLine) {
			scrollToLine(previousFirstLine, Scroll::alignTop, firstVisibleLineFraction);
		}
	}

	miniMap.update(config, document, typeSetter);

	// handle possible delayed change callback
	if (document.isUpdated()) {
		if (delayedChangeCallback && !delayedChangeDetected) {
			delayedChangeDetected = true;
			delayedChangeReportTime = std::chrono::system_clock::now() + delayedChangeDelay;
		}
	}

	// compress marker and squiggle lists (if required)
	if (deletesHappened) {
		if (markers.size()) {
			compressMarkers();
		}

		if (squiggles.size()) {
			compressSquiggles();
		}

		deletesHappened = false;
	}

	// remember if document was changed this frame
	const bool documentChanged = document.isUpdated();

	// reset overlay "dirty" flags
	document.resetUpdated();
	bracketeer.resetUpdated();
	lineFold.resetUpdated();
	typeSetter.resetUpdated();

	// get "new" total editor size in pixels
	const auto width =
		textLeftOffset +
		typeSetter.getColumnCount() * glyphSize.x +
		cursorWidth +
		(config.showMiniMap ? miniMapWidth : 0.0f);

	const auto height = typeSetter.getRowCount() * glyphSize.y;
	totalSize = ImVec2(width, height);

	return documentChanged;
}


//
//	TextEditor::handleKeyboardInputs
//

void TextEditor::handleKeyboardInputs() {
	if (ImGui::IsWindowFocused()) {
		auto& io = ImGui::GetIO();
		const auto macOS = io.ConfigMacOSXBehaviors;

		// ignore specific keys when autocomplete is active, they will be handled later
		if (autocomplete.isActive() && autocomplete.isSpecialKeyPressed()) {
			if (autocomplete.hasSuggestions()) {
				return;

			} else {
				// cancel autocomplete when special keys are used without any suggestions
				autocomplete.cancel();
			}
		}

		// cursor movements and selections
		if (ImGui::Shortcut(ImGuiKey_UpArrow, ImGuiInputFlags_Repeat)) { moveUp(1, false); }
		else if (ImGui::Shortcut(ImGuiMod_Shift | ImGuiKey_UpArrow, ImGuiInputFlags_Repeat)) { moveUp(1, true); }
		else if (ImGui::Shortcut(ImGuiKey_DownArrow, ImGuiInputFlags_Repeat)) { moveDown(1, false); }
		else if (ImGui::Shortcut(ImGuiMod_Shift | ImGuiKey_DownArrow, ImGuiInputFlags_Repeat)) { moveDown(1, true); }

		else if (ImGui::Shortcut((macOS ? ImGuiMod_Super : ImGuiMod_Alt) | ImGuiMod_Shift | ImGuiKey_LeftArrow)) { shrinkSelections(); }
		else if (ImGui::Shortcut((macOS ? ImGuiMod_Super : ImGuiMod_Alt) | ImGuiMod_Shift | ImGuiKey_RightArrow)) { growSelections(); }

		else if (ImGui::Shortcut(ImGuiKey_LeftArrow, ImGuiInputFlags_Repeat)) { moveLeft(false, false); }
		else if (ImGui::Shortcut(ImGuiMod_Shift | ImGuiKey_LeftArrow, ImGuiInputFlags_Repeat)) { moveLeft(true, false); }
		else if (ImGui::Shortcut((macOS ? ImGuiMod_Alt : ImGuiMod_Ctrl) | ImGuiKey_LeftArrow, ImGuiInputFlags_Repeat)) { moveLeft(false, true); }
		else if (ImGui::Shortcut((macOS ? ImGuiMod_Alt : ImGuiMod_Ctrl) | ImGuiMod_Shift | ImGuiKey_LeftArrow, ImGuiInputFlags_Repeat)) { moveLeft(true, true); }

		else if (ImGui::Shortcut(ImGuiKey_RightArrow, ImGuiInputFlags_Repeat)) { moveRight(false, false); }
		else if (ImGui::Shortcut(ImGuiMod_Shift | ImGuiKey_RightArrow, ImGuiInputFlags_Repeat)) { moveRight(true, false); }
		else if (ImGui::Shortcut((macOS ? ImGuiMod_Alt : ImGuiMod_Ctrl) | ImGuiKey_RightArrow, ImGuiInputFlags_Repeat)) { moveRight(false, true); }
		else if (ImGui::Shortcut((macOS ? ImGuiMod_Alt : ImGuiMod_Ctrl) | ImGuiMod_Shift | ImGuiKey_RightArrow, ImGuiInputFlags_Repeat)) { moveRight(true, true); }

		else if (ImGui::Shortcut(ImGuiKey_PageUp, ImGuiInputFlags_Repeat)) { moveUp(lastVisibleRow - firstVisibleRow - 2, false); }
		else if (ImGui::Shortcut(ImGuiMod_Shift | ImGuiKey_PageUp, ImGuiInputFlags_Repeat)) { moveUp(lastVisibleRow - firstVisibleRow - 2, true); }
		else if (ImGui::Shortcut(ImGuiKey_PageDown, ImGuiInputFlags_Repeat)) { moveDown(lastVisibleRow - firstVisibleRow - 2, false); }
		else if (ImGui::Shortcut(ImGuiMod_Shift | ImGuiKey_PageDown, ImGuiInputFlags_Repeat)) { moveDown(lastVisibleRow - firstVisibleRow - 2, true); }

		else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_UpArrow)) { moveToTop(false); }
		else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Home)) { moveToTop(false); }
		else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_UpArrow)) { moveToTop(true); }
		else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Home)) { moveToTop(true); }

		else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_DownArrow)) { moveToBottom(false); }
		else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_End)) { moveToBottom(false); }
		else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_DownArrow)) { moveToBottom(true); }
		else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_End)) { moveToBottom(true); }

		else if (ImGui::Shortcut(ImGuiKey_Home)) { moveToStartOfLine(false); }
		else if (ImGui::Shortcut(ImGuiMod_Shift | ImGuiKey_Home)) { moveToStartOfLine(true); }

		else if (ImGui::Shortcut(ImGuiKey_End)) { moveToEndOfLine(false); }
		else if (ImGui::Shortcut(ImGuiMod_Shift | ImGuiKey_End)) { moveToEndOfLine(true); }

		else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_A)) { selectAll(); }
		else if (cursors.currentCursorHasSelection() && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_D)) { addNextOccurrence(false); }
		else if (cursors.currentCursorHasSelection() && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Alt | ImGuiKey_D)) { addNextOccurrence(true); }
		else if (cursors.currentCursorHasSelection() && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Alt | ImGuiKey_L)) { addNextOccurrence(true); }
		else if (cursors.currentCursorHasSelection() && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_D)) { selectAllOccurrences(false); }
		else if (cursors.currentCursorHasSelection() && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Alt | ImGuiMod_Shift | ImGuiKey_D)) { selectAllOccurrences(true); }

		// clipboard operations
		else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_X)) { cut(); }

		else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_C)) { copy(); }
		else if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Insert)) { copy(); }

		else if (!config.readOnly && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_V, ImGuiInputFlags_Repeat)) { paste(); }
		else if (!config.readOnly && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Insert, ImGuiInputFlags_Repeat)) { paste(); }

		else if (!config.readOnly && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z, ImGuiInputFlags_Repeat)) { undo(); }
		else if (!config.readOnly && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z, ImGuiInputFlags_Repeat)) { redo(); }
		else if (!config.readOnly && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Y, ImGuiInputFlags_Repeat)) { redo(); }

		// remove text
		else if (!config.readOnly && ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_Repeat)) { handleDelete(false); }
		else if (!config.readOnly && ImGui::Shortcut(ImGuiMod_Shift | ImGuiKey_Delete, ImGuiInputFlags_Repeat)) { handleDelete(false); }
		else if (!config.readOnly && ImGui::Shortcut((macOS ? ImGuiMod_Alt : ImGuiMod_Ctrl) | ImGuiKey_Delete, ImGuiInputFlags_Repeat)) { handleDelete(true); }

		else if (!config.readOnly && ImGui::Shortcut(ImGuiKey_Backspace, ImGuiInputFlags_Repeat)) { handleBackspace(false); }
		else if (!config.readOnly && ImGui::Shortcut(ImGuiMod_Shift | ImGuiKey_Backspace, ImGuiInputFlags_Repeat)) { handleBackspace(false); }
		else if (!config.readOnly && ImGui::Shortcut((macOS ? ImGuiMod_Alt : ImGuiMod_Ctrl) | ImGuiKey_Backspace, ImGuiInputFlags_Repeat)) { handleBackspace(true); }

		else if (!config.readOnly && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_K)) { removeSelectedLines(); }

		// text manipulation
		else if (!config.readOnly && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_LeftBracket, ImGuiInputFlags_Repeat)) { deindentLines(); }
		else if (!config.readOnly && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_RightBracket, ImGuiInputFlags_Repeat)) { indentLines(); }

		else if (!config.readOnly && ImGui::Shortcut(ImGuiMod_Alt | ImGuiKey_UpArrow)) { moveUpLines(); }
		else if (!config.readOnly && ImGui::Shortcut(ImGuiMod_Alt | ImGuiKey_DownArrow)) { moveDownLines(); }

		else if (!config.readOnly && config.language && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Slash)) { toggleComments(); }
		else if (!config.readOnly && config.language && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_L)) { toggleComments(); }

		// find/replace support
		else if (config.findReplaceEnabled && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_F)) {
			if (autocomplete.isActive()) {
				autocomplete.cancel();
				findCancelledAutocomplete = true;
			}

			openFindReplace();
		}

		else if (config.findReplaceEnabled && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_F)) { findAll(); }
		else if (config.findReplaceEnabled && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_G, ImGuiInputFlags_Repeat)) { findNext(); }

		// autocomplete support
		else if (!config.readOnly && ImGui::Shortcut(autocomplete.getTriggerShortcut())) {
			// don't activate if we have multiple cursors active
			if (cursors.hasMultiple()) {
				// TODO: inform user

			} else {
				if (autocomplete.startShortcut(cursors)) {
					makeCursorVisible();
				}
			}
		}

		// change insert mode
		else if (ImGui::Shortcut(ImGuiKey_Insert)) { config.overwrite = !config.overwrite; }

		// handle new line
		else if (!config.readOnly && ImGui::Shortcut(ImGuiKey_Enter, ImGuiInputFlags_Repeat) ) { handleCharacter('\n'); }
		else if (!config.readOnly && ImGui::Shortcut(ImGuiKey_KeypadEnter, ImGuiInputFlags_Repeat)) { handleCharacter('\n'); }

		else if (!config.readOnly && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Enter, ImGuiInputFlags_Repeat)) { insertLineBelow(); }
		else if (!config.readOnly && ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_KeypadEnter, ImGuiInputFlags_Repeat)) { insertLineBelow(); }

		else if (!config.readOnly && ImGui::Shortcut(ImGuiMod_Shift | ImGuiKey_Enter, ImGuiInputFlags_Repeat)) { insertLineAbove(); }
		else if (!config.readOnly && ImGui::Shortcut(ImGuiMod_Shift | ImGuiKey_KeypadEnter, ImGuiInputFlags_Repeat)) { insertLineAbove(); }

		// handle tabs
		else if (!config.readOnly && ImGui::Shortcut(ImGuiKey_Tab, ImGuiInputFlags_Repeat)) {
			if (cursors.anyHasSelection()) {
				indentLines();

			} else {
				handleCharacter('\t');
			}
		}

		else if (!config.readOnly && ImGui::Shortcut(ImGuiMod_Shift | ImGuiKey_Tab, ImGuiInputFlags_Repeat)) {
				deindentLines();
		}

		// handle escape key
		else if ((autocomplete.isActive() || findReplaceVisible || cursors.hasMultiple()) && ImGui::Shortcut(ImGuiKey_Escape)) {
			if (autocomplete.isActive()) {
				autocomplete.cancel();

			} else if (findReplaceVisible) {
				closeFindReplace();

			} else if (cursors.hasMultiple()) {
				cursors.clearAdditional();
			}
		}

		// handle regular text
		if (!io.InputQueueCharacters.empty()) {
			// ignore Ctrl inputs, but need to allow Alt+Ctrl as some keyboards (e.g. German) use AltGR (which is Alt+Ctrl) to input certain characters
			if (!(ImGui::IsKeyDown(ImGuiMod_Ctrl) && !ImGui::IsKeyDown(ImGuiMod_Alt)) && !config.readOnly) {
				for (auto i = 0; i < io.InputQueueCharacters.size(); i++) {
					const auto character = io.InputQueueCharacters[i];

					if (character == '\n' || character >= 32) {
						handleCharacter(character);
					}
				}
			}

			io.InputQueueCharacters.resize(0);
		}

		// notify OS of text input position for advanced Input Method Editor (IME)
		// this is required for the SDL3 backend as it will not report text input events unless we do this
		// see https://github.com/ocornut/imgui/issues/8584 for details
		if (!config.readOnly) {
			auto context = ImGui::GetCurrentContext();
			context->PlatformImeData.WantVisible = true;
			context->PlatformImeData.WantTextInput = true;

			// at the main cursor: the top left of its line (as in Dear ImGui's InputText)
			const auto cursorPos = visPos2ScreenPos(docPos2VisPos(getCursorPosition(cursors.getMainIndex())));
			context->PlatformImeData.InputPos = ImVec2(cursorPos.x - 1.0f, cursorPos.y);
			context->PlatformImeData.InputLineHeight = glyphSize.y;
			context->PlatformImeData.ViewportId = ImGui::GetCurrentWindow()->Viewport->ID;
		}

		// tell app to ignore keyboard inputs
		ImGui::SetNextFrameWantCaptureKeyboard(true);
	}
}


//
//	TextEditor::handleMouseInteractions
//

void TextEditor::handleMouseInteractions() {
	auto io = ImGui::GetIO();
	const auto mousePos = ImGui::GetMousePos() - cursorScreenPos;
	const auto absoluteMousePos = ImGui::GetMousePos() - ImGui::GetWindowPos();

	const auto overLineNumbers =
		config.showLineNumbers &&
		(absoluteMousePos.x > lineNumberLeftOffset) &&
		(absoluteMousePos.x < lineNumberRightOffset);

	const auto overText =
		mousePos.x - ImGui::GetScrollX() > textLeftOffset &&
		mousePos.x - ImGui::GetScrollX() < textRightOffset &&
		mousePos.y - ImGui::GetScrollY() >= 0 &&
		mousePos.y - ImGui::GetScrollY() < textSize.y;

	const auto overMiniMap = config.showMiniMap && absoluteMousePos.x > miniMapOffset;

	DocPos glyphPos;
	DocPos cursorPos;

	typeSetter.screenPos2DocPos(
		document,
		ImVec2((mousePos.x - textLeftOffset) / glyphSize.x, mousePos.y / glyphSize.y),
		glyphPos,
		cursorPos);

	panning &= config.panMode && ImGui::IsMouseDown(ImGuiMouseButton_Middle);

	// handle middle mouse button panning
	if (panning && ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
		const auto windowSize = ImGui::GetWindowSize();
		auto mouseDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
		const float dragFactor = ImGui::GetIO().DeltaTime * 15.0f;
		const ImVec2 autoPanMargin(glyphSize.x * 4.0f, glyphSize.y * 2.0f);

		if (absoluteMousePos.x < textLeftOffset + autoPanMargin.x) {
			mouseDelta.x = (absoluteMousePos.x - (textLeftOffset + autoPanMargin.x)) * dragFactor;

		} else if (absoluteMousePos.x > windowSize.x - autoPanMargin.x) {
			mouseDelta.x = (absoluteMousePos.x - (windowSize.x - autoPanMargin.x)) * dragFactor;
		}

		if (absoluteMousePos.y < autoPanMargin.y) {
			mouseDelta.y = (absoluteMousePos.y - autoPanMargin.y) * dragFactor;

		} else if (absoluteMousePos.y > windowSize.y - autoPanMargin.y) {
			mouseDelta.y = (absoluteMousePos.y - (windowSize.y - autoPanMargin.y)) * dragFactor;
		}

		ImGui::SetScrollX(ImGui::GetScrollX() - mouseDelta.x);
		ImGui::SetScrollY(ImGui::GetScrollY() - mouseDelta.y);
		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);

	// handle middle mouse button scrolling
	} else if (scrolling) {
		const float deadzone = glyphSize.x;
		auto offset = scrollStart - absoluteMousePos;
		offset.x = (offset.x < 0.0f) ? std::min(offset.x + deadzone, 0.0f) : std::max(offset.x - deadzone, 0.0f);
		offset.y = (offset.y < 0.0f) ? std::min(offset.y + deadzone, 0.0f) : std::max(offset.y - deadzone, 0.0f);

		const float scrollFactor = ImGui::GetIO().DeltaTime * 5.0f;
		offset *= scrollFactor;

		ImGui::SetScrollX(ImGui::GetScrollX() - offset.x);
		ImGui::SetScrollY(ImGui::GetScrollY() - offset.y);

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) ||
			ImGui::IsMouseClicked(ImGuiMouseButton_Middle) ||
			ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {

			scrolling = false;
		}

	// handle left mouse button dragging
	} else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
		// update selection with dragging left mouse button
		io.WantCaptureMouse = true;

		if (miniMapIsScrollbar) {
			const auto localOffset = absoluteMousePos.y - miniMapScrollStart;
			const auto documentOffset = localOffset * totalSize.y / textSize.y;
			auto scrollY = miniMapScrollY + documentOffset;
			scrollY = std::clamp(scrollY, 0.0f, totalSize.y - textSize.y);
			ImGui::SetScrollY(scrollY);

		} else if (selectingText && overLineNumbers) {
			auto& cursor = cursors.getCurrent();
			const auto start = DocPos(cursorPos.line, 0);
			const auto end = normalizePos(DocPos(cursorPos.line + 1, 0));
			cursor.update(cursor.getInteractiveEnd() < cursor.getInteractiveStart() ? start : end);
			makeCursorVisible();

		} else if (selectingText && overText) {
			cursors.updateCurrentCursor(cursorPos);
			makeCursorVisible();
		}

	// end minimap scroll (if required)
	} else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
		miniMapIsScrollbar = false;
		selectingText = false;

	// ignore other interactions when the editor is not hovered
	} else if (ImGui::IsWindowHovered()) {
		// show text cursor if required
		if (ImGui::IsWindowFocused() && overText) {
			ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
		}

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
			// start panning/scrolling mode on middle mouse click
			if (config.panMode) {
				panning = true;

			} else {
				scrolling = true;
				scrollStart = absoluteMousePos;
			}

		} else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
			// handle right clicks by setting up context menu (if required)
			if (overLineNumbers && lineNumberContextMenuCallback) {
				popupDocPos = glyphPos;
				ImGui::OpenPopup("LineNumberContextMenu");

			} else if (overText && textContextMenuCallback) {
				popupDocPos = glyphPos;
				ImGui::OpenPopup("TextContextMenu");
			}

		} else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			// handle left mouse button actions
			selectingText = overText || overLineNumbers;
			const auto doubleClick = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
			const auto now = static_cast<float>(ImGui::GetTime());
			const auto tripleClick = !doubleClick && lastClickTime != -1.0f && (now - lastClickTime) < io.MouseDoubleClickTime;
			lastClickTime = tripleClick ? -1.0f : now;

			if (tripleClick) {
				// left mouse button triple click
				if (overText) {
					const auto start = document.getStartOfLine(cursorPos);
					const auto end = normalizePos(DocPos(start.line + 1, 0));
					cursors.updateCurrentCursor(start, end);
				}

			} else if (doubleClick) {
				// left mouse button double click
				if (overText) {
					const auto codepoint = document.getCodePoint(glyphPos);
					bool handled = false;

					// select bracketed section (if required)
					if (CodePoint::isBracketOpener(codepoint)) {
						const auto brackets = bracketeer.getEnclosingBrackets(document.getRight(glyphPos));

						if (brackets != bracketeer.end()) {
							if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
								cursors.setCursor(brackets->start, document.getRight(brackets->end));

							} else {
								cursors.setCursor(document.getRight(brackets->start), brackets->end);
							}

							handled = true;
						}

					} else if (CodePoint::isBracketCloser(codepoint)) {
						const auto brackets = bracketeer.getEnclosingBrackets(glyphPos);

						if (brackets != bracketeer.end()) {
							cursors.setCursor(brackets->start, document.getRight(brackets->end));
							handled = true;
						}
					}

					// select "word" if it wasn't a bracketed section
					// run of whitespaces are considered "words" as are operator sequences
					if (!handled && !document.isEndOfLine(glyphPos)) {
						const auto word = document.getWholeWord(glyphPos);

						if (word.start != word.end) {
							cursors.updateCurrentCursor(word.start, word.end);
						}
					}
				}

			} else {
				// left mouse button single click
				const auto extendCursor = ImGui::IsKeyDown(ImGuiMod_Shift);

				const auto addCursor = ImGui::GetIO().ConfigMacOSXBehaviors
					? ImGui::IsKeyDown(ImGuiMod_Alt)
					: ImGui::IsKeyDown(ImGuiMod_Ctrl);

				if (overLineNumbers) {
					// handle line number clicks
					const auto start = DocPos(cursorPos.line, 0);
					const auto end = normalizePos(DocPos(cursorPos.line + 1, 0));

					if (extendCursor) {
						auto& cursor = cursors.getCurrent();
						cursor.update(cursor.getInteractiveEnd() < cursor.getInteractiveStart() ? start : end);
						autocomplete.cancel();

					} else if (addCursor) {
						cursors.addCursor(start, end);
						autocomplete.cancel();

					} else {
						cursors.setCursor(start, end);
					}

					makeCursorVisible();

				} else if (overText) {
					// handle mouse clicks in text
					if (extendCursor) {
						cursors.growCurrentCursor(cursorPos);
						autocomplete.cancel();

					} else if (addCursor) {
						cursors.addCursor(cursorPos);
						autocomplete.cancel();

					} else {
						cursors.setCursor(cursorPos);
					}

					makeCursorVisible();

				} else if (overMiniMap) {
					if (ImGui::GetCurrentWindow()->ScrollbarY) {
						const auto clickedRow = firstMiniMapRow + static_cast<size_t>(absoluteMousePos.y / miniMapRowHeight);

						if (clickedRow < firstVisibleRow || clickedRow > lastVisibleRow) {
							scrollToLine(visPos2DocPos(VisPos(clickedRow, 0)).line, Scroll::alignMiddle);

						} else {
							miniMapIsScrollbar = true;
							miniMapScrollStart = absoluteMousePos.y;
							miniMapScrollY = ImGui::GetScrollY();
						}
					}
				}
			}

		} else if (textHoverCallback && IsMousePosOverGlyph(ImGui::GetMousePos())) {
			// capture position for text hover popup
			popupDocPos = document.findWordStart(glyphPos, true);
			const auto vizPos = docPos2VisPos(popupDocPos);

			popupWindowPos = ImVec2(
				vizPos.column * glyphSize.x + textLeftOffset + cursorScreenPos.x,
				vizPos.row * glyphSize.y + cursorScreenPos.y);

			// only open popup if required
			if (!ImGui::IsPopupOpen("TextHoverPopup")) {
				ImGui::OpenPopup("TextHoverPopup");
			}
		}

		// update cursors and reset vertical navigation if any button was pressed
		bool anyMouseDown = false;

		for (size_t i = 0; !anyMouseDown && i < static_cast<size_t>(ImGuiMouseButton_Middle); i++) {
			if (ImGui::IsMouseDown(static_cast<ImGuiMouseButton>(i))) {
				anyMouseDown = true;
			}
		}

		if (anyMouseDown) {
			cursors.update(document);
			navigatingVertically = false;
		}
	}
}


//
//	TextEditor::isDocPosVisible
//

bool TextEditor::isDocPosVisible(DocPos docPos) const {
	if (isLineHidden(docPos.line)) {
		return false;

	} else {
		const auto visPos = docPos2VisPos(docPos);

		return
			visPos.row >= firstVisibleRow && visPos.row < lastVisibleRow &&
			visPos.column >= firstVisibleColumn && visPos.column < lastVisibleColumn;
	}
}


//
//	TextEditor::selectAll
//

void TextEditor::selectAll() {
	moveToTop(false);
	moveToBottom(true);
}


//
//	TextEditor::selectLine
//

void TextEditor::selectLine(size_t line) {
	moveTo(DocPos(line, 0), false);
	moveTo(normalizePos(DocPos(line + 1, 0)), true);
}


//
//	TextEditor::selectLines
//

void TextEditor::selectLines(size_t startLine, size_t endLine) {
	moveTo(DocPos(startLine, 0), false);
	moveTo(DocPos(endLine, 0), true);
}


//
//	TextEditor::selectRegion
//

void TextEditor::selectRegion(DocPos start, DocPos end) {
	if (end < start) {
		std::swap(start, end);
	}

	cursors.setCursor(start, end);
}


//
//	TextEditor::selectToBrackets
//

void TextEditor::selectToBrackets(bool includeBrackets) {
	for (auto& cursor : cursors) {
		const auto bracket = bracketeer.getEnclosingBrackets(cursor.getSelectionStart());

		if (bracket != bracketeer.end()) {
			if (includeBrackets) {
				cursor.update(bracket->start, document.getRight(bracket->end));

			} else {
				cursor.update(document.getRight(bracket->start), bracket->end);
			}
		}
	}
}


//
//	TextEditor::growSelections
//

void TextEditor::growSelections() {
	for (auto& cursor : cursors) {
		if (cursor.hasSelection()) {
			const auto start = cursor.getSelectionStart();
			const auto end = cursor.getSelectionEnd();
			const auto startCodePoint = document.getCodePoint(document.getLeft(start));
			const auto endCodePoint = document.getCodePoint(end);

			if (CodePoint::isBracketOpener(startCodePoint) && endCodePoint == CodePoint::toPairCloser(startCodePoint)) {
				cursor.update(document.getLeft(start),document.getRight(end));

			} else {
				const auto bracket = bracketeer.getEnclosingBrackets(start, end);

				if (bracket != bracketeer.end()) {
					cursor.update(document.getRight(bracket->start), bracket->end);
				}
			}

		} else {
			const auto pos = cursor.getSelectionEnd();
			const auto start = document.findWordStart(pos);
			const auto end = document.findWordEnd(pos);
			cursor.update(start, end);
		}
	}
}


//
//	TextEditor::shrinkSelections
//

void TextEditor::shrinkSelections() {
	for (auto& cursor : cursors) {
		if (cursor.hasSelection()){
			const auto start = cursor.getSelectionStart();
			const auto end = cursor.getSelectionEnd();
			const auto startCodePoint = document.getCodePoint(start);
			const auto endCodePoint = document.getCodePoint(document.getLeft(end));

			if (CodePoint::isBracketOpener(startCodePoint) && endCodePoint == CodePoint::toPairCloser(startCodePoint)) {
				cursor.update(document.getRight(start),document.getLeft(end));

			} else {
				const auto bracket = bracketeer.getInnerBrackets(start, end);

				if (bracket != bracketeer.end()) {
					cursor.update(bracket->start, document.getRight(bracket->end));
				}
			}
		}
	}
}


//
//	TextEditor::cut
//

void TextEditor::cut() {
	// copy selections to clipboard and remove them
	copy();
	const auto transaction = startTransaction();
	deleteTextFromAllCursors(transaction);
	cursors.getCurrent().resetToStart();
	endTransaction(transaction);
}


//
//	TextEditor::copy
//

void TextEditor::copy() const {
	// copy all selections and put them on the clipboard
	// empty cursors copy the entire line
	std::string text;

	if (cursors.anyHasSelection()) {
		for (const auto& cursor : cursors) {
			if (text.size()) {
				text += "\n";
			}

			if (cursor.hasSelection()) {
				text += document.getSectionText(cursor.getSelectionStart(), cursor.getSelectionEnd());

			} else {
				text += document.getLineText(cursor.getSelectionStart().line);
			}
		}

	} else {
		for (const auto& cursor : cursors) {
			text += document.getLineText(cursor.getSelectionStart().line) + "\n";
		}
	}

	ImGui::SetClipboardText(text.c_str());
}


//
//	TextEditor::paste
//

void TextEditor::paste() {
	// ignore non-text clipboard content
	auto clipboard = ImGui::GetClipboardText();

	if (clipboard) {
		const auto transaction = startTransaction();
		insertTextIntoAllCursors(transaction, clipboard);
		endTransaction(transaction);
	}
}


//
//	TextEditor::undo
//

void TextEditor::undo() {
	if (transactions.canUndo()) {
		transactions.undo(config, document, cursors);
		makeCursorVisible();
	}
}


//
//	TextEditor::redo
//

void TextEditor::redo() {
	if (transactions.canRedo()) {
		transactions.redo(config, document, cursors);
		makeCursorVisible();
	}
}


//
//	TextEditor::getCursorPosition
//

TextEditor::DocPos TextEditor::getCursorPosition(size_t cursor) const {
	cursor = std::min(cursor, cursors.size() - 1);
	return cursors[cursor].getInteractiveEnd();
}


//
//	TextEditor::getCursor
//

TextEditor::DocSelection TextEditor::getCursorSelection(size_t cursor) const {
	cursor = std::min(cursor, cursors.size() - 1);
	const auto start = cursors[cursor].getSelectionStart();
	const auto end = cursors[cursor].getSelectionEnd();
	return DocSelection(start, end);
}


//
//	TextEditor::isMousePosOverGlyph
//

bool TextEditor::isMousePosOverGlyph(const ImVec2& mousePos) const {
	if (firstFrame) {
		return false;

	} else {
		// convert mouse position to screen coordinates
		const auto local = mousePos - cursorScreenPos;

		// ignore negative coordinates
		if (local.x < 0.0f || local.y < 0.0f) {
			return false;
		}

		// convert to visual position and check it
		const VisPos visPos(static_cast<size_t>(local.y / glyphSize.y), static_cast<size_t>((local.x - textLeftOffset) / glyphSize.x));
		return typeSetter.isVisPosOverGlyph(visPos);
	}
}


//
//	TextEditor::isMousePosOverTextArea
//

bool TextEditor::isMousePosOverTextArea(const ImVec2& mousePos) const {
	if (firstFrame) {
		return false;

	} else {
		// convert mouse position to screen coordinates
		const auto local = mousePos - cursorScreenPos;
		return local.x > textLeftOffset && local.x < textRightOffset && local.y >= 0 && local.y < textSize.y;
	}
}


//
//	TextEditor::getDocPosAtMousePos
//

TextEditor::DocPos TextEditor::getDocPosAtMousePos(const ImVec2& mousePos) const {
	if (firstFrame) {
		return DocPos();

	} else {
		// convert mouse position to screen coordinates
		auto local = mousePos - cursorScreenPos;

		// ignore negative coordinates
		if (local.y < 0.0f) {
			return DocPos(0, 0);

		} else if (local.x < 0.0f) {
			local.x = 0.0f;
		}

		// convert to document position
		const VisPos visPos(static_cast<size_t>(local.y / glyphSize.y), static_cast<size_t>((local.x - textLeftOffset) / glyphSize.x));
		return visPos2DocPos(normalizePos(visPos));
	}
}


//
//	TextEditor::getWordAtMousePos
//

std::string TextEditor::getWordAtMousePos(const ImVec2& mousePos) const {
	if (IsMousePosOverGlyph(mousePos)) {
		// convert to document position
		const DocPos docPos = getDocPosAtMousePos(mousePos);

		// Find word boundaries and extract text
		const auto word = document.getWholeWord(docPos, true);
		return document.getSectionText(word.start, word.end);

	} else {
		return "";
	}
}


//
//	TextEditor::setCursor
//

void TextEditor::setCursor(DocPos pos) {
	scrollToDocPos = normalizePos(pos);
	cursors.setCursor(scrollToDocPos);

	if (config.lineFolding) {
		lineFold.unfoldAroundLine(document, scrollToDocPos.line);
	}
}


//
//	TextEditor::scrollToLine
//

void TextEditor::scrollToLine(size_t line, Scroll alignment, float fraction) {
	scrollToDocPos = DocPos(invalidLine, 0);
	scrollToLineNumber = normalizeLine(line);
	scrollToAlignment = alignment;
	scrollToFraction = fraction;

	if (config.lineFolding) {
		lineFold.unfoldAroundLine(document, line);
	}
}


//
//	TextEditor::handlePossibleScrolling
//

void TextEditor::handlePossibleScrolling() {
	// apply scrolling (if needed)
	float scrollX = -1.0f;
	float scrollY = -1.0f;

	// when some API calls are made, requests are stored since the Dear ImGui context is only valid during rendering
	// a user could change text after scrolling (in the same frame) which could make the position invalid
	// this is why the document positions are normalized again here

	// do we need to make a certain position visible
	if (scrollToDocPos.line != invalidLine) {
		if (lastVisibleRow == 0 && lastVisibleColumn == 0) {
			// this is a hack to handle a chicken and egg issue when the user called SetCursor before the first frame
			lastVisibleRow = std::min(static_cast<size_t>(std::ceil(textSize.y / glyphSize.y)), typeSetter.getRowCount() - 1);
			lastVisibleColumn = static_cast<size_t>(std::ceil(textSize.x / glyphSize.x));
		}

		const auto pos = docPos2VisPos(normalizePos(scrollToDocPos));

		if (pos.row <= firstVisibleRow + 1) {
			scrollY = std::max(0.0f, (pos.row - 2.0f) * glyphSize.y);

		} else if (pos.row >= lastVisibleRow - 1) {
			scrollY = std::max(0.0f, (pos.row + 2.0f) * glyphSize.y - textSize.y);
		}

		if (pos.column <= firstVisibleColumn + 1) {
			scrollX = std::max(0.0f, (pos.column - 2.0f) * glyphSize.x);

		} else if (pos.column >= lastVisibleColumn - 1) {
			scrollX = std::max(0.0f, (pos.column + 2.0f) * glyphSize.x - textSize.x);
		}

		scrollToDocPos.line = invalidLine;
	}

	// scroll to specified line (if required)
	if (scrollToLineNumber != invalidLine) {
		const auto row = static_cast<float>(docPos2VisPos(DocPos(normalizeLine(scrollToLineNumber), 0)).row);
		const auto visibleRows = textSize.y / glyphSize.y;
		scrollX = 0.0f;

		switch (scrollToAlignment) {
			case Scroll::alignTop:
				scrollY = (row + scrollToFraction) * glyphSize.y;
				break;

			case Scroll::alignMiddle:
				scrollY = std::max(0.0f, (row - visibleRows / 2.0f) * glyphSize.y);
				break;

			case Scroll::alignBottom:
				scrollY = std::max(0.0f, (row - (visibleRows - 1.0f)) * glyphSize.y);
				break;
		}

		scrollToLineNumber = invalidLine;
	}

	// set scroll (if required)
	if (scrollX >= 0.0f) {
		ImGui::SetScrollX(scrollX);
	}

	if (scrollY >= 0.0f) {
		if (config.smoothScrolling) {
			smoothScrollTargetY = scrollY;
			smoothScrollTime = 0.0f;

		} else {
			ImGui::SetScrollY(scrollY);
			smoothScrollTargetY = -1.0f;
		}
	}

	// running smooth scroll: a fast ease toward its target, which it reaches after smoothScrollDuration
	// the user's own scrolling stops it
	if (smoothScrollTargetY >= 0.0f) {
		smoothScrollTime += ImGui::GetIO().DeltaTime;

		if (ImGui::IsWindowHovered() && ImGui::GetIO().MouseWheel != 0.0f) {
			smoothScrollTargetY = -1.0f;

		} else if (smoothScrollTime >= smoothScrollDuration) {
			ImGui::SetScrollY(smoothScrollTargetY);
			smoothScrollTargetY = -1.0f;

		} else {
			const auto current = ImGui::GetScrollY();
			const auto step = 1.0f - std::exp(-ImGui::GetIO().DeltaTime * 18.0f);
			ImGui::SetScrollY(current + (smoothScrollTargetY - current) * step);
		}
	}
}


//
//	TextEditor::makeCursorVisible
//

void TextEditor::makeCursorVisible() {
	scrollToDocPos = cursors.getCurrent().getInteractiveEnd();
	scrollToLineNumber = invalidLine;

	if (config.lineFolding) {
		lineFold.unfoldAroundLine(document, scrollToDocPos.line);
	}

	resetCursorAnimationTimer();
}


//
//	TextEditor::resetScrolling
//

void TextEditor::resetScrolling() {
	// reset state related parameters
	scrollToDocPos = DocPos(invalidLine, 0);
	scrollToLineNumber = invalidLine;
	firstVisibleRow = 0;
	lastVisibleRow = 0;
	firstVisibleColumn = 0;
	lastVisibleColumn = 0;
	resetCursorAnimationTimer();

	// we can't reset the Dear ImGui scrolling here as we don't necessarily have the right context
	// so we just set a flag and the render function will handle it
	resetDearImGuiScrolling = true;
}


//
//	TextEditor::addMarker
//

void TextEditor::addMarker(size_t line, ImU32 lineNumberColor, ImU32 textColor, const std::string_view& lineNumberTooltip, const std::string_view& textTooltip) {
	if (line < document.size()) {
		markers.emplace_back(lineNumberColor, textColor, lineNumberTooltip, textTooltip);
		document[line].marker = markers.size();
	}
}


//
//	TextEditor::clearMarkers
//

void TextEditor::clearMarkers() {
	for (auto& line : document) {
		line.marker = 0;
	}

	markers.clear();
}


//
//	TextEditor::compressMarkers
//

void TextEditor::compressMarkers() {
	if (markers.size()) {
		// references to current markers
		struct Reference {
			bool used = false;
			bool index = 0;
		};

		std::vector<Reference> references(markers.size());

		// determine markers still in use
		for (const auto& line : document) {
			if (line.marker) {
				references[line.marker - 1].used = true;
			}
		}

		// reindex markers
		size_t index = 0;

		for (auto& reference : references) {
			if (reference.used) {
				reference.index = index++;
			}
		}

		// apply new index numbers to lines
		for (auto& line : document) {
			if (line.marker) {
				line.marker = references[line.marker - 1].index + 1;
			}
		}

		// remove unused markers
		auto i = markers.size();

		do {
			i--;

			if (!references[i].used) {
				markers.erase(markers.begin() + i);
			}

		} while (i > 0);
	}
}


//
//	TextEditor::addSquiggle
//

void TextEditor::addSquiggle(DocPos start, DocPos end, size_t type, ImU32 color, const std::string_view& tooltip, SquiggleStyle style) {
	if (start < end) {
		squiggles.emplace_back(type, color, tooltip, style);
		const auto index = squiggles.size();

		document.iterateGlyphs(start, end, [index](Glyph& glyph) {
			glyph.squiggle = static_cast<uint32_t>(index);
		});
	}
}


//
//	TextEditor::clearSquiggles
//

void TextEditor::clearSquiggles(size_t type) {
	for (auto& line : document) {
		for (auto& glyph : line) {
			if (glyph.squiggle) {
				if (squiggles[glyph.squiggle - 1].type == type) {
					glyph.squiggle = 0;
				}
			}
		}
	}

	compressSquiggles();
}


//
//	TextEditor::clearSquiggles
//

void TextEditor::clearSquiggles(DocPos start, DocPos end) {
	document.iterateGlyphs(start, end, [](Glyph& glyph) {
		glyph.squiggle = 0;
	});

	compressSquiggles();
}


//
//	TextEditor::clearSquiggles
//

void TextEditor::clearSquiggles() {
	for (auto& line : document) {
		for (auto& glyph : line) {
			glyph.squiggle = 0;
		}
	}

	squiggles.clear();
}


//
//	TextEditor::compressSquiggles
//

void TextEditor::compressSquiggles() {
	if (squiggles.size()) {
		// references to current squiggles;
		struct Reference {
			bool used = false;
			bool index = 0;
		};

		std::vector<Reference> references(squiggles.size());

		// determine squiggles still in use
		for (const auto& line : document) {
			for (const auto& glyph : line) {
				if (glyph.squiggle) {
					references[glyph.squiggle - 1].used = true;
				}
			}
		}

		// reindex squiggles
		size_t index = 0;

		for (auto& reference : references) {
			if (reference.used) {
				reference.index = index++;
			}
		}

		// apply new index numbers to glyphs
		for (auto& line : document) {
			for (auto& glyph : line) {
				if (glyph.squiggle) {
					glyph.squiggle = references[glyph.squiggle - 1].index + 1;
				}
			}
		}

		// remove unused squiggles
		auto i = squiggles.size();

		do {
			i--;

			if (!references[i].used) {
				squiggles.erase(squiggles.begin() + i);
			}

		} while (i > 0);
	}
}


//
//	TextEditor::moveUp
//

void TextEditor::moveUp(size_t rows, bool select) {
	// these must be done with visual coordinates
	for (auto& cursor : cursors) {
		auto visPos = docPos2VisPos(cursor.getInteractiveEnd());

		if (navigatingVertically) {
			visPos.column = cursor.getPreferredColumn();

		} else {
			cursor.setPreferredColumn(visPos.column);
		}

		if (rows > visPos.row) {
			visPos.row = 0;

		} else {
			visPos.row -= rows;
		}

		visPos = normalizePos(visPos);
		cursor.update(visPos2DocPos(visPos), select);
	}

	makeCursorVisible();
	navigatingVertically = true;
}


//
//	TextEditor::moveDown
//

void TextEditor::moveDown(size_t rows, bool select) {
	// these must be done with visual coordinates
	for (auto& cursor : cursors) {
		auto visPos = docPos2VisPos(cursor.getInteractiveEnd());

		if (navigatingVertically) {
			visPos.column = cursor.getPreferredColumn();

		} else {
			cursor.setPreferredColumn(visPos.column);
		}

		visPos.row += rows;
		visPos = normalizePos(visPos);
		cursor.update(visPos2DocPos(visPos), select);
	}

	makeCursorVisible();
	navigatingVertically = true;
}


//
//	TextEditor::moveLeft
//

void TextEditor::moveLeft(bool select, bool wordMode) {
	for (auto& cursor : cursors) {
		if (cursor.hasSelection() && !select && !wordMode) {
			cursor.resetToStart();

		} else {
			cursor.update(document.getLeft(cursor.getInteractiveEnd(), wordMode), select);
		}
	}

	makeCursorVisible();
	navigatingVertically = false;
}


//
//	TextEditor::moveRight
//

void TextEditor::moveRight(bool select, bool wordMode) {
	for (auto& cursor : cursors) {
		if (cursor.hasSelection() && !select && !wordMode) {
			cursor.resetToEnd();

		} else {
			cursor.update(document.getRight(cursor.getInteractiveEnd(), wordMode), select);
		}
	}

	makeCursorVisible();
	navigatingVertically = false;
}


//
//	TextEditor::moveTo
//

void TextEditor::moveTo(DocPos coordinate, bool select) {
	cursors.clearAdditional();
	cursors.updateCurrentCursor(coordinate, select);
	makeCursorVisible();
	navigatingVertically = false;
}


//
//	TextEditor::handleCharacter
//

void TextEditor::handleCharacter(ImWchar character) {
	const auto transaction = startTransaction(false);

	const auto opener = character;
	const auto isPaired = !config.overwrite && config.completePairedGlyphs && CodePoint::isPairOpener(opener);
	const auto closer = CodePoint::toPairCloser(opener);

	// ignore input if it was the closing character for a pair that was automatically inserted
	if (completePairCloser) {
		if (completePairCloser == character && completePairLocation == cursors.getCurrent().getSelectionEnd()) {
			completePairCloser = 0;
			moveRight(false, false);
			return;
		}

		completePairCloser = 0;
	}

	if (cursors.anyHasSelection() && isPaired) {
		// encapsulate the current selections with the requested pairs
		for (auto cursor = cursors.begin(); cursor < cursors.end(); cursor++) {
			if (cursor->hasSelection()) {
				const auto start = cursor->getSelectionStart();
				const auto end = cursor->getSelectionEnd();

				// insert the closing glyph
				char utf8[4];
				const auto end1 = insertText(transaction, end, std::string_view(utf8, CodePoint::write(utf8, closer)));
				cursors.adjustForInsert(cursor, start, end1);

				// insert the opening glyph
				const auto end2 = insertText(transaction, start, std::string_view(utf8, CodePoint::write(utf8, opener)));
				cursors.adjustForInsert(cursor, start, end2);

				// update old selection
				cursor->update(document.getRight(start), document.getRight(end));
			}
		}

	} else if (isPaired) {
		// insert the requested pair
		char utf8[8];
		auto size = CodePoint::write(utf8, opener);
		size += CodePoint::write(utf8 + size, closer);
		const std::string_view pair(utf8, size);

		for (auto cursor = cursors.begin(); cursor < cursors.end(); cursor++) {
			const auto start = cursor->getSelectionStart();
			const auto end = insertText(transaction, start, pair);
			cursors.adjustForInsert(cursor, start, end);
			cursor->update(document.getRight(start), false);
		}

		// remember the closer
		completePairCloser = closer;
		completePairLocation = cursors.getCurrent().getSelectionEnd();

	} else if (!config.overwrite && config.autoIndent && character == '\n') {
		// handle auto indent case
		autoIndentAllCursors(transaction);

	} else {
		// handle overwrite by deleting next glyph before insert
		if (config.overwrite) {
			for (auto cursor = cursors.begin(); cursor < cursors.end(); cursor++) {
				if (!cursor->hasSelection()) {
					const auto start = cursor->getSelectionStart();

					if (start != document.getEndOfLine(start)) {
						const auto end = document.getRight(start);
						deleteText(transaction, start, end);
						cursors.adjustForDelete(cursor, start, end);
					}
				}
			}
		}

		// just insert a regular character
		char utf8[4];
		insertTextIntoAllCursors(transaction, std::string_view(utf8, CodePoint::write(utf8, character)));
	}

	endTransaction(transaction);

	if (!cursors.hasMultiple() && autocomplete.startTyping(cursors, character)) {
		makeCursorVisible();
	}
}


//
//	TextEditor::handleBackspace
//

void TextEditor::handleBackspace(bool wordMode) {
	const auto transaction = startTransaction(false);

	// remove selections or characters to the left of the cursor
	for (auto cursor = cursors.begin(); cursor < cursors.end(); cursor++) {
		const auto start = cursor->hasSelection() ? cursor->getSelectionStart() : document.getLeft(cursor->getSelectionStart(), wordMode);
		const auto end = cursor->getSelectionEnd();
		deleteText(transaction, start, end);
		cursor->update(start, false);
		cursors.adjustForDelete(cursor, start, end);
	}

	endTransaction(transaction);
}


//
//	TextEditor::handleDelete
//

void TextEditor::handleDelete(bool wordMode) {
	const auto transaction = startTransaction(false);

	// remove selections or characters to the right of the cursor
	for (auto cursor = cursors.begin(); cursor < cursors.end(); cursor++) {
		const auto start = cursor->getSelectionStart();
		const auto end = cursor->hasSelection() ? cursor->getSelectionEnd() : document.getRight(cursor->getSelectionEnd(), wordMode);
		deleteText(transaction, start, end);
		cursor->update(start, false);
		cursors.adjustForDelete(cursor, start, end);
	}

	endTransaction(transaction);
}


//
//	TextEditor::removeSelectedLines
//

void TextEditor::removeSelectedLines() {
	const auto transaction = startTransaction();

	for (auto cursor = cursors.begin(); cursor < cursors.end(); cursor++) {
		const auto start = document.getStartOfLine(cursor->getSelectionStart());
		auto end = cursor->getSelectionEnd();
		end = (end.index == 0) ? end : document.getNextLine(end);
		deleteText(transaction, start, end);
		cursor->update(start, false);
		cursors.adjustForDelete(cursor, start, end);
	}

	endTransaction(transaction);
}


//
//	TextEditor::insertLineAbove
//

void TextEditor::insertLineAbove() {
	const auto transaction = startTransaction();

	for (auto cursor = cursors.begin(); cursor < cursors.end(); cursor++) {
		const auto start = document.getStartOfLine(cursor->getSelectionStart());
		const auto end = insertText(transaction, start, "\n");
		cursor->update(start, false);
		cursors.adjustForInsert(cursor, start, end);
	}

	endTransaction(transaction);
}


//
//	TextEditor::insertLineBelow
//

void TextEditor::insertLineBelow() {
	const auto transaction = startTransaction();

	for (auto cursor = cursors.begin(); cursor < cursors.end(); cursor++) {
		auto start = cursor->getSelectionEnd();
		start = (start.index == 0) ? start : document.getNextLine(start);
		const auto end = insertText(transaction, start, "\n");
		cursor->update(start, false);
		cursors.adjustForInsert(cursor, start, end);
	}

	endTransaction(transaction);
}


//
//	TextEditor::indentLines
//

void TextEditor::indentLines() {
	const auto transaction = startTransaction();

	// process all cursors
	for (auto cursor = cursors.begin(); cursor < cursors.end(); cursor++) {
		const auto cursorStart = cursor->getSelectionStart();
		const auto cursorEnd = cursor->getSelectionEnd();

		// process all lines in this cursor
		for (auto line = cursorStart.line; line <= cursorEnd.line; line++) {
			if ((!cursor->hasSelection() || DocPos(line, 0) != cursorEnd) && document[line].size()) {
				const auto insertStart = DocPos(line, 0);
				const auto insertEnd = insertText(transaction, insertStart, "\t");
				cursors.adjustForInsert(cursor, insertStart, insertEnd, true);
			}
		}
	}

	endTransaction(transaction);
}


//
//	TextEditor::deindentLines
//

void TextEditor::deindentLines() {
	const auto transaction = startTransaction();

	// process all cursors
	for (auto cursor = cursors.begin(); cursor < cursors.end(); cursor++) {
		const auto cursorStart = cursor->getSelectionStart();
		const auto cursorEnd = cursor->getSelectionEnd();

		for (auto line = cursorStart.line; line <= cursorEnd.line; line++) {
			if ((!cursor->hasSelection() || DocPos(line, 0) != cursorEnd) && document[line].size()) {
				// determine how many whitespaces are available at the start with a max of tabSize columns
				size_t column = 0;
				size_t index = 0;

				while (column < config.tabSize && index < document[line].size() && std::isblank(document[line][index].codepoint)) {
					column += document[line][index].columns;
					index++;
				}

				// delete that whitespace (if required)
				const DocPos deleteStart(line, 0);
				const DocPos deleteEnd(line, index);

				if (deleteEnd != deleteStart) {
					deleteText(transaction, deleteStart, deleteEnd);
					cursors.adjustForDelete(cursor, deleteStart, deleteEnd, true);
				}
			}
		}
	}

	endTransaction(transaction);
}


//
//	Widget::moveUpLines
//

void TextEditor::moveUpLines() {
	// don't move up if first line is in one of the cursors
	if (cursors[0].getSelectionStart().line != 0) {
		const auto transaction = startTransaction();

		for (auto cursor = cursors.begin(); cursor < cursors.end(); cursor++) {
			const auto start = cursor->getSelectionStart();
			const auto end = cursor->getSelectionEnd();

			// delete existing lines
			const auto deleteStart = document.getStartOfLine(start);
			const auto deleteEnd = (end.index == 0) ? end : document.getNextLine(end);
			const auto text = document.getSectionText(deleteStart, deleteEnd);
			deleteText(transaction, deleteStart, deleteEnd);
			cursors.adjustForDelete(cursor, deleteStart, deleteEnd);

			// insert text one line up
			const DocPos insertStart(deleteStart.line - 1, 0);
			const auto insertEnd = insertText(transaction, insertStart, text);
			cursors.adjustForInsert(cursor, insertStart, insertEnd);

			// update cursor
			cursor->update(start - DocPos(1, 0), end - DocPos(1, 0));
		}

		endTransaction(transaction);
	}
}


//
//	TextEditor::moveDownLines
//

void TextEditor::moveDownLines() {
	// don't move up if last line is in one of the cursors
	if (!document.isLastLine(cursors[cursors.size() - 1].getSelectionStart().line)) {
		const auto transaction = startTransaction();

		for (auto cursor = cursors.begin(); cursor < cursors.end(); cursor++) {
			const auto start = cursor->getSelectionStart();
			const auto end = cursor->getSelectionEnd();

			// delete existing lines
			const auto deleteStart = document.getStartOfLine(start);
			const auto deleteEnd = (end.index == 0) ? end : document.getNextLine(end);
			const auto text = document.getSectionText(deleteStart, deleteEnd);
			deleteText(transaction, deleteStart, deleteEnd);
			cursors.adjustForDelete(cursor, deleteStart, deleteEnd);

			// insert text one line down
			const auto insertStart = document.getNextLine(deleteStart);
			const auto insertEnd = insertText(transaction, insertStart, text);
			cursors.adjustForInsert(cursor, insertStart, insertEnd);

			// update cursor
			cursor->update(start + DocPos(1, 0), end + DocPos(1, 0));
		}

		endTransaction(transaction);
	}
}


//
//	TextEditor::toggleComments
//

void TextEditor::toggleComments() {
	const auto transaction = startTransaction();
	const auto comment = config.language->singleLineComment;

	// process all cursors
	for (auto cursor = cursors.begin(); cursor < cursors.end(); cursor++) {
		const auto cursorStart = cursor->getSelectionStart();
		const auto cursorEnd = cursor->getSelectionEnd();

		// process all lines in this cursor
		for (auto line = cursorStart.line; line <= cursorEnd.line; line++) {
			if ((!cursor->hasSelection() || DocPos(line, 0) != cursorEnd) && document[line].size()) {
				// see if line starts with a comment (after possible leading whitespaces)
				size_t start = 0;
				size_t i = 0;

				while (start < document[line].size() && CodePoint::isWhiteSpace(document[line][start].codepoint)) {
					start++;
				}

				while (start + i < document[line].size() && i < comment.size() && document[line][start + i].codepoint == static_cast<ImWchar>(comment[i])) {
					i++;
				}

				if (i == comment.size()) {
					const auto deleteStart = DocPos(line, start);
					auto endOfComment = start + i;

					if (endOfComment < document[line].size() - 1 && document[line][endOfComment].codepoint == ' ') {
						endOfComment++;
					}

					const auto deleteEnd = DocPos(line, endOfComment);
					deleteText(transaction, deleteStart, deleteEnd);
					cursors.adjustForDelete(cursor, deleteStart, deleteEnd, true);

				} else {
					const auto insertStart = DocPos(line, start);
					const auto insertEnd = insertText(transaction, insertStart, comment + " ");
					cursors.adjustForInsert(cursor, insertStart, insertEnd, true);
				}
			}
		}
	}

	endTransaction(transaction);
}


//
//	TextEditor::filterSelections
//

void TextEditor::filterSelections(std::function<std::string(std::string_view)> filter) {
	const auto transaction = startTransaction();

	// process all cursors
	for (auto cursor = cursors.begin(); cursor < cursors.end(); cursor++) {
		const auto start = cursor->getSelectionStart();
		const auto end = cursor->getSelectionEnd();

		// process all lines in this cursor
		for (auto line = start.line; line <= end.line; line++) {
			if (DocPos(line, 0) != end && document[line].size()) {
				// get original text and run it through filter
				const auto before = document.getSectionText(start, end);
				const std::string after = filter(before);

				// update selection if anything changed
				if (after != before) {
					deleteText(transaction, start, end);
					cursors.adjustForDelete(cursor, start, end);
					const auto newEnd = insertText(transaction, start, after);
					cursor->update(start, newEnd);
					cursors.adjustForInsert(cursor, start, newEnd);
				}
			}
		}
	}

	endTransaction(transaction);
}


//
//	TextEditor::selectionToLowerCase
//

void TextEditor::selectionToLowerCase() {
	FilterSelections([](const std::string_view& text) {
		std::string result;
		const auto end = text.end();
		auto i = text.begin();
		char utf8[4];

		while (i < end) {
			ImWchar codepoint;
			i = CodePoint::read(i, end, &codepoint);
			result.append(utf8, CodePoint::write(utf8, CodePoint::toLower(codepoint)));
		}

		return result;
	});
}


//
//	TextEditor::selectionToUpperCase
//

void TextEditor::selectionToUpperCase() {
	FilterSelections([](const std::string_view& text) {
		std::string result;
		const auto end = text.end();
		auto i = text.begin();
		char utf8[4];

		while (i < end) {
			ImWchar codepoint;
			i = CodePoint::read(i, end, &codepoint);
			result.append(utf8, CodePoint::write(utf8, CodePoint::toUpper(codepoint)));
		}

		return result;
	});
}


//
//	TextEditor::stripTrailingWhitespaces
//

void TextEditor::stripTrailingWhitespaces() {
	const auto transaction = startTransaction();

	// process all the lines
	for (size_t i = 0; i < document.size(); i++) {
		const auto& line = document[i];
		const auto lineSize = line.size();
		auto whitespace = std::numeric_limits<size_t>::max();

		// look for first non-whitespace glyph at the end of the line
		if (lineSize) {
			bool done = false;

			for (auto index = lineSize - 1; !done; index--) {
				if (CodePoint::isWhiteSpace(line[index].codepoint)) {
					whitespace = index;

					if (index == 0) {
						done = true;
					}

				} else {
					done = true;
				}
			}
		}

		// remove whitespaces (if required)
		if (whitespace != std::numeric_limits<size_t>::max()) {
			deleteText(transaction, DocPos(i, whitespace), DocPos(i, lineSize));
		}
	}

	// update cursor if transaction wasn't empty
	if (endTransaction(transaction)) {
		cursors.setCursor(normalizePos(cursors.getCurrent().getSelectionEnd()));
	}
}


//
//	TextEditor::filterLines
//

void TextEditor::filterLines(std::function<std::string(std::string_view)> filter) {
	const auto transaction = startTransaction();

	// process all the lines
	for (size_t i = 0; i < document.size(); i++) {
		// get original text and run it through filter
		const auto before = document.getLineText(i);
		const auto after = filter(before);

		// update line if anything changed
		if (after != before) {
			const auto start = DocPos(i, 0);
			const auto end = document.getEndOfLine(start);
			deleteText(transaction, start, end);
			insertText(transaction, start, after);
		}
	}

	// update cursor if transaction wasn't empty
	if (endTransaction(transaction)) {
		cursors.setCursor(normalizePos(cursors.getCurrent().getSelectionEnd()));
	}
}


//
//	TextEditor::tabsToSpaces
//

void TextEditor::tabsToSpaces() {
	filterLines([this](const std::string_view& input) {
		std::string output;
		const auto end = input.end();
		auto i = input.begin();
		size_t columns = 0;

		while (i < end) {
			ImWchar codepoint;
			i = CodePoint::read(i, end, &codepoint);

			if (codepoint == '\t') {
				const auto spaces = config.tabSize - (columns % config.tabSize);
				output.append(spaces, ' ');
				columns += spaces;

			} else {
				char utf8[4];
				output.append(utf8, CodePoint::write(utf8, codepoint));
				columns += CodePoint::getGlyphWidth(codepoint);
			}
		}

		return output;
	});
}


//
//	TextEditor::spacesToTabs
//

void TextEditor::spacesToTabs() {
	FilterLines([this](const std::string_view& input) {
		std::string output;
		const auto end = input.end();
		auto i = input.begin();
		size_t columns = 0;
		size_t spaces = 0;

		while (i < end) {
			ImWchar codepoint;
			i = CodePoint::read(i, end, &codepoint);

			if (codepoint == ' ') {
				spaces++;

			} else {
				while (spaces) {
					const auto spacesUntilNextTab = config.tabSize - (columns % config.tabSize);

					if (spacesUntilNextTab == 1) {
						output += ' ';
						columns++;
						spaces--;

					} else if (spaces >= spacesUntilNextTab) {
						output += '\t';
						columns += spacesUntilNextTab;
						spaces -= spacesUntilNextTab;

					} else if (codepoint != '\t')
						while (spaces) {
							output += ' ';
							columns++;
							spaces--;
						}

					else {
						spaces = 0;
					}
				}

				if (codepoint == '\t') {
					output += '\t';
					columns += config.tabSize - (columns % config.tabSize);

				} else {
					char utf8[4];
					output.append(utf8, CodePoint::write(utf8, codepoint));
					columns += CodePoint::getGlyphWidth(codepoint);
				}
			}
		}

		return output;
	});
}


//
//	TextEditor::startTransaction
//

std::shared_ptr<TextEditor::Transaction> TextEditor::startTransaction(bool cancelsAutoComplete) {
	if (cancelsAutoComplete) {
		autocomplete.cancel();
	}

	std::shared_ptr<Transaction> transaction = Transactions::create();
	transaction->setBeforeState(cursors);
	return transaction;
}


//
//	TextEditor::endTransaction
//

bool TextEditor::endTransaction(std::shared_ptr<Transaction> transaction) {
	if (transaction->size() > 0) {
		cursors.update(document);
		transaction->setAfterState(cursors);
		transactions.add(transaction);
		return true;

	} else {
		return false;
	}
}


//
//	TextEditor::insertTextIntoAllCursors
//

void TextEditor::insertTextIntoAllCursors(std::shared_ptr<Transaction> transaction, const std::string_view& text) {
	// delete any selection content first
	deleteTextFromAllCursors(transaction);

	// insert the text
	for (auto cursor = cursors.begin(); cursor < cursors.end(); cursor++) {
		const auto start = cursor->getSelectionStart();
		const auto end = insertText(transaction, start, text);
		cursor->update(end, false);
		cursors.adjustForInsert(cursor, start, end);
	}
}


//
//	TextEditor::deleteTextFromAllCursors
//

void TextEditor::deleteTextFromAllCursors(std::shared_ptr<Transaction> transaction) {
	for (auto cursor = cursors.begin(); cursor < cursors.end(); cursor++) {
		if (cursor->hasSelection()) {
			const auto start = cursor->getSelectionStart();
			const auto end = cursor->getSelectionEnd();
			deleteText(transaction, start, end);
			cursor->update(start, false);
			cursors.adjustForDelete(cursor, start, end);
		}
	}
}


//
//	TextEditor::autoIndentAllCursors
//

void TextEditor::autoIndentAllCursors(std::shared_ptr<Transaction> transaction) {
	for (auto cursor = cursors.begin(); cursor < cursors.end(); cursor++) {
		const auto start = cursor->getSelectionStart();

		// delete any selections
		if (cursor->hasSelection()) {
			const auto end = cursor->getSelectionEnd();
			deleteText(transaction, start, end);
			cursors.adjustForDelete(cursor, start, end);
		}

		// get previous and next character
		auto index = start.index;
		const auto& line = document[start.line];
		const ImWchar previousChar = index > 0 ? line[index - 1].codepoint : 0;
		const ImWchar nextChar = index < line.size() ? line[index].codepoint : 0;

		// remove extra whitespaces if required
		if (CodePoint::isWhiteSpace(nextChar)) {
			while (index < line.size() && CodePoint::isWhiteSpace(line[index].codepoint)) {
				index++;
			}

			const auto end = DocPos(start.line, index);
			deleteText(transaction, start, end);
			cursors.adjustForDelete(cursor, start, end);
		}

		// determine whitespace at start of current line
		std::string whitespace;

		for (size_t i = 0; i < line.size() && CodePoint::isWhiteSpace(line[i].codepoint); i++) {
			char utf8[4];
			whitespace.append(utf8, CodePoint::write(utf8, line[i].codepoint));
		}

		// determine text to insert
		std::string insert = "\n" + whitespace;
		auto newCursorIndex = whitespace.size();

		// handle special cases
		if (previousChar == CodePoint::openCurlyBracket || previousChar == CodePoint::openSquareBracket) {
			if (config.insertSpacesOnTabs) {
				for (size_t i = 0; i < config.tabSize; i++) {
					insert += " ";
				}

				newCursorIndex += config.tabSize;

			} else {
				insert += "\t";
				newCursorIndex++;
			}

			if ((previousChar == CodePoint::openCurlyBracket && nextChar == CodePoint::closeCurlyBracket) ||
				(previousChar == CodePoint::openSquareBracket && nextChar == CodePoint::closeSquareBracket)) {

				// open a new block
				insert += "\n" + whitespace;
			}
		}

		// insert new text
		const auto end = insertText(transaction, start, insert);
		cursors.adjustForInsert(cursor, start, end);

		// set new cursor location
		cursor->update(DocPos(start.line + 1, newCursorIndex), false);
	}
}


//
//	TextEditor::insertText
//

TextEditor::DocPos TextEditor::insertText(std::shared_ptr<Transaction> transaction, DocPos start, const std::string_view& text) {
	// update document, add transaction and return position of end of insert
	// this function does not change the cursors
	const auto end = document.insertText(config, start, text);
	transaction->addInsert(start, end, text);
	makeCursorVisible();
	return end;
}


//
//	TextEditor::deleteText
//

void TextEditor::deleteText(std::shared_ptr<Transaction> transaction, DocPos start, DocPos end) {
	// update document and add transaction
	// this function does not change the cursors
	const auto text = document.getSectionText(start, end);
	document.deleteText(config, start, end);
	transaction->addDelete(start, end, text);
	makeCursorVisible();
	resetCursorAnimationTimer();
	deletesHappened = true;
}


//
//	TextEditor::Cursor::grow
//

void TextEditor::Cursor::grow(DocPos position) {
	if (position < getSelectionStart()) {
		if (end > start) {
			std::swap(end, start);
		}

		end = position;
		updated = true;

	} else if (position > getSelectionEnd()) {
		if (end < start) {
			std::swap(end, start);
		}

		end = position;
		updated = true;
	}
}


//
//	TextEditor::Cursor::adjustCoordinateForInsert
//

TextEditor::DocPos TextEditor::Cursor::adjustCoordinateForInsert(DocPos position, DocPos insertStart, DocPos insertEnd) {
	if (position.line == insertStart.line) {
		position.index += insertEnd.index - insertStart.index;
	}

	position.line += insertEnd.line - insertStart.line;
	return position;
}


//
//	TextEditor::Cursor::adjustForInsert
//

void TextEditor::Cursor::adjustForInsert(DocPos insertStart, DocPos insertEnd) {
	start = adjustCoordinateForInsert(start, insertStart, insertEnd);
	end = adjustCoordinateForInsert(end, insertStart, insertEnd);
}


//
//	TextEditor::Cursor::adjustCoordinateForDelete
//

TextEditor::DocPos TextEditor::Cursor::adjustCoordinateForDelete(DocPos position, DocPos deleteStart, DocPos deleteEnd) {
	if (deleteStart.line == deleteEnd.line) {
		if (position.line == deleteEnd.line) {
			position.index -= deleteEnd.index - deleteStart.index;
		}

	} else {
		position.line -= deleteEnd.line - deleteStart.line;

		if (position.line == deleteEnd.line) {
			position.index -= deleteEnd.index;
		}
	}

	return position;
}


//
//	TextEditor::Cursor::adjustForDelete
//

void TextEditor::Cursor::adjustForDelete(DocPos deleteStart, DocPos deleteEnd) {
	start = adjustCoordinateForDelete(start, deleteStart, deleteEnd);
	end = adjustCoordinateForDelete(end, deleteStart, deleteEnd);
}


//
//	TextEditor::Cursor::ensureNotHidden
//

void TextEditor::Cursor::ensureNotHidden(const Document& document) {
	if (document[start.line].foldingState == FoldingState::hidden) {
		auto line = start.line - 1;

		while (document[line].foldingState == FoldingState::hidden) {
			line--;
		}

		start.line = line;
		start.index = document[line].size();
		updated = true;
	}

	if (document[end.line].foldingState == FoldingState::hidden) {
		auto line = end.line - 1;

		while (document[line].foldingState == FoldingState::hidden) {
			line--;
		}

		end.line = line;
		end.index = document[line].size();
		updated = true;
	}
}


//
//	TextEditor::Cursors::reset
//

void TextEditor::Cursors::reset() {
	clear();
	main = 0;
	current = 0;
}


//
//	TextEditor::Cursors::setCursor
//

void TextEditor::Cursors::setCursor(DocPos cursorStart, DocPos cursorEnd) {
	reset();
	emplace_back(cursorStart, cursorEnd);
	front().setMain(true);
	front().setCurrent(true);
}


//
//	TextEditor::Cursors::addCursor
//

void TextEditor::Cursors::addCursor(DocPos start, DocPos end) {
	at(current).setCurrent(false);
	emplace_back(start, end);
	back().setCurrent(true);
	current = size() - 1;
}


//
//	TextEditor::Cursors::anyHasSelection
//

bool TextEditor::Cursors::anyHasSelection() const {
	for (auto cursor = begin(); cursor < end(); cursor++) {
		if (cursor->hasSelection()) {
			return true;
		}
	}

	return false;
}


//
//	TextEditor::Cursors::allHaveSelection
//

bool TextEditor::Cursors::allHaveSelection() const {
	for (auto cursor = begin(); cursor < end(); cursor++) {
		if (!cursor->hasSelection()) {
			return false;
		}
	}

	return true;
}


//
//	TextEditor::Cursors::anyHasUpdate
//

bool TextEditor::Cursors::anyHasUpdate() const {
	for (auto cursor = begin(); cursor < end(); cursor++) {
		if (cursor->isUpdated()) {
			return true;
		}
	}

	return false;
}


//
//	TextEditor::Cursors::clearAll
//

void TextEditor::Cursors::clearAll() {
	reset();
	emplace_back(DocPos(0, 0));
	front().setMain(true);
	front().setCurrent(true);
}


//
//	TextEditor::Cursors::clearAdditional
//

void TextEditor::Cursors::clearAdditional(bool reset) {
	for (auto cursor = begin(); cursor < end();) {
		if (cursor->isMain()) {
			cursor++;

		} else {
			cursor = erase(cursor);
		}
	}

	main = 0;
	current = 0;
	front().setCurrent(true);

	if (reset) {
		front().resetToEnd();
	}
}


//
//	TextEditor::Cursors::clearUpdated
//

void TextEditor::Cursors::clearUpdated() {
	for (auto cursor = begin(); cursor < end(); cursor++) {
		cursor->setUpdated(false);
	}
}


//
//	TextEditor::Cursors::update
//

void TextEditor::Cursors::update(const Document& document) {
	// ensure cursors are not on hidden lines
	for (auto& cursor : *this) {
		cursor.ensureNotHidden(document);
	}

	if (anyHasUpdate()) {
		// reset update flags
		clearUpdated();

		//  only sort and potential merge when we have multiple cursors
		if (hasMultiple()) {
			// sort cursors
			std::sort(begin(), end(), [](Cursor& a, Cursor& b) {
				return a.getSelectionStart() < b.getSelectionStart();
			});

			// merge cursors
			for (auto cursor = rbegin(); cursor < rend() - 1;) {
				auto previous = cursor + 1;

				if (previous->getSelectionEnd() >= cursor->getSelectionEnd()) {
					// handle case where one cursor completely contains another cursor
					if (cursor->isMain()) {
						previous->setMain(true);
					}

					if (cursor->isCurrent()) {
						previous->setCurrent(true);
					}

					cursor = std::reverse_iterator(erase(previous.base()));

				} else if (previous->getSelectionEnd() > cursor->getSelectionStart()) {
					if (cursor->getInteractiveEnd() < cursor->getInteractiveStart()) {
						previous->update(cursor->getSelectionEnd(), previous->getSelectionStart());

					} else {
						previous->update(previous->getSelectionStart(), cursor->getSelectionEnd());
					}

					if (cursor->isMain()) {
						previous->setMain(true);
					}

					if (cursor->isCurrent()) {
						previous->setCurrent(true);
					}

					cursor = std::reverse_iterator(erase(previous.base()));

				} else {
					cursor++;
				}
			}

			// find main and current cursor
			main = 0;
			current = 0;

			for (size_t c = 0; c < size(); c++) {
				if (at(c).isMain()) {
					main = c;

				} else if (at(c).isCurrent()) {
					current = c;
				}
			}
		}
	}
}


//
//	TextEditor::Cursors::adjustForInsert
//

void TextEditor::Cursors::adjustForInsert(iterator start, DocPos insertStart, DocPos insertEnd, bool includeCurrent) {
	auto first = includeCurrent ? start : start + 1;

	for (auto cursor = first; cursor < end(); cursor++) {
		cursor->adjustForInsert(insertStart, insertEnd);
	}
}


//
//	TextEditor::Cursors::adjustForDelete
//

void TextEditor::Cursors::adjustForDelete(iterator start, DocPos deleteStart, DocPos deleteEnd, bool includeCurrent) {
	auto first = includeCurrent ? start : start + 1;

	for (auto cursor = first; cursor < end(); cursor++) {
		cursor->adjustForDelete(deleteStart, deleteEnd);
	}
}


//
//	TextEditor::Document::insertText
//

TextEditor::DocPos TextEditor::Document::insertText(const Config& config, DocPos start, const std::string_view& text) {
	auto line = begin() + start.line;
	auto index = start.index;
	auto lineNo = start.line;

	// process input as UTF-8
	const auto endOfText = text.end();
	auto i = text.begin();

	// process all codepoints
	while (i < endOfText) {
		ImWchar character;
		i = CodePoint::read(i, endOfText, &character);

		if (character == '\n') {
			// split line
			insertLine(lineNo + 1);
			line = begin() + lineNo;
			const auto nextLine = begin() + ++lineNo;

			for (auto j = line->begin() + index; j < line->end(); j++) {
				nextLine->push_back(*j);
			}

			line->erase(line->begin() + index, line->end());
			line = nextLine;
			index = 0;

		} else if (config.insertSpacesOnTabs && character == '\t') {
			const auto spaces = getSpacesToTab(config, *line, index);

			for (size_t s = 0; s < spaces; s++) {
				line->insert(line->begin() + (index++), Glyph(' ', Color::text));
			}

		} else if (character != '\r') {
			// insert next glyph
			line->insert(line->begin() + (index++), Glyph(character, Color::text));
		}
	}

	// determine end of insert
	const auto end = DocPos(lineNo, index);

	// mark affected lines as changed
	for (auto j = start.line; j <= end.line; j++) {
		at(j).needsColorizing = true;
		at(j).needsTypeSetting = true;
	}

	// calculate line indents
	updateIndents(config, start.line, end.line);
	updated = true;
	return end;
}


//
//	TextEditor::Document::deleteText
//

void TextEditor::Document::deleteText(const Config& config, DocPos start, DocPos end) {
	auto& startLine = at(start.line);
	auto& endLine = at(end.line);
	const auto startIndex = start.index;
	const auto endIndex = end.index;

	// see if start and end are on the same line
	if (start.line == end.line) {
		startLine.erase(startLine.begin() + startIndex, startLine.begin() + endIndex);

	// start and end are on different lines
	} else {
		// remove end of first line
		startLine.erase(startLine.begin() + startIndex, startLine.end());

		// remove start of last line
		endLine.erase(endLine.begin(), endLine.begin() + endIndex);

		// join lines
		startLine.insert(startLine.end(), endLine.begin(), endLine.end());

		// delete lines
		deleteLines(start.line + 1, end.line);
	}

	// remove marker
	startLine.marker = 0;

	// mark line and document as changed
	at(start.line).needsColorizing = true;
	at(start.line).needsTypeSetting = true;

	// calculate line indents
	updateIndents(config, start.line, start.line);
	updated = true;
}


//
//	TextEditor::Document::getSectionText
//

std::string TextEditor::Document::getSectionText(DocPos start, DocPos end) const {
	std::string section;

	auto lineNo = start.line;
	auto index = start.index;
	char utf8[4];

	while (lineNo < end.line || index < end.index) {
		const auto& line = at(lineNo);

		if (index < line.size()) {
			section.append(std::string_view(utf8, CodePoint::write(utf8, line[index].codepoint)));
			index++;

		} else {
			section += '\n';
			lineNo++;
			index = 0;
		}
	}

	return section;
}


//
//	TextEditor::Document::getLineCodePoints
//

std::vector<ImWchar> TextEditor::Document::getLineCodePoints(size_t lineNo) const {
	auto& line = at(lineNo);
	std::vector<ImWchar> result;
	result.reserve(line.size());

	for (auto& glyph : line) {
		result.emplace_back(glyph.codepoint);
	}

	return result;
}


//
//	TextEditor::Document::getCodePoint
//

ImWchar TextEditor::Document::getCodePoint(DocPos location) const {
	if (location.index < at(location.line).size()) {
		return at(location.line)[location.index].codepoint;

	} else {
		return IM_UNICODE_CODEPOINT_INVALID;
	}
}


//
//	TextEditor::Document::iterateGlyphs
//

void TextEditor::Document::iterateGlyphs(DocPos start, DocPos end, std::function<void(Glyph&)> callback) {
	if (start.line == end.line) {
		// start and end are on same line
		for (size_t i = start.index; i < end.index; i++) {
			callback(at(start.line)[i]);
		}

	} else {
		// process remainder of fist line
		auto last = at(start.line).size();

		for (size_t i = start.index; i < last; i++) {
			callback(at(start.line)[i]);
		}

		// process all full lines
		for (auto line = start.line + 1; line < end.line; line++) {
			last = at(line).size();

			for (size_t i = 0; i < last; i++) {
				callback(at(line)[i]);
			}
		}

		// process remainder on last line
		for (size_t i = 0; i < end.index; i++) {
			callback(at(end.line)[i]);
		}
	}
}


//
//	TextEditor::Document::getColor
//

TextEditor::Color TextEditor::Document::getColor(DocPos location)  const {
	if (location.index < at(location.line).size()) {
		return at(location.line)[location.index].color;

	} else {
		return Color::text;
	}
}


//
//	TextEditor::Document::getLeft
//

TextEditor::DocPos TextEditor::Document::getLeft(DocPos from, bool wordMode) const {
	if (wordMode) {
		// move to the end of the previous line if we're at the start of a line
		if (from.index == 0) {
			from = getLeft(from);
		}

		// now skip all whitespaces
		from = findPreviousNonWhiteSpace(from, false);

		// find the start of the current word
		return findWordStart(from);

	} else {
		// calculate position of previous glyph (could be on previous line)
		if (from.index == 0) {
			return getLeftBeforeHiddenLines(from);

		} else {
			return DocPos(from.line, from.index - 1);
		}
	}
}


//
//	TextEditor::Document::getRight
//

TextEditor::DocPos TextEditor::Document::getRight(DocPos from, bool wordMode) const {
	if (wordMode) {
		// move to the start of the next line if we're at the end of a line
		if (from.index == at(from.line).size()) {
			from = getRight(from);
		}

		// now skip all whitespaces
		from = findNextNonWhiteSpace(from, false);

		// find the end of the current word
		return findWordEnd(from);

	} else {
		// calculate pos of next glyph (could be on next line)
		if (from.index == at(from.line).size()) {
			return getRightAfterHiddenLines(from);

		} else {
			return DocPos(from.line, from.index + 1);
		}
	}
}


//
//	TextEditor::Document::getTop
//

TextEditor::DocPos TextEditor::Document::getTop() const {
	return DocPos(0, 0);
}


//
//	TextEditor::Document::getBottom
//

TextEditor::DocPos TextEditor::Document::getBottom() const {
	const auto lastLine = size() - 1;
	return DocPos(lastLine, at(lastLine).size());
}


//
//	TextEditor::Document::getStartOfLine
//

TextEditor::DocPos TextEditor::Document::getStartOfLine(DocPos from) const {
	return DocPos(from.line, 0);
}


//
//	TextEditor::Document::getEndOfLine
//

TextEditor::DocPos TextEditor::Document::getEndOfLine(DocPos from) const {
	return DocPos(from.line, at(from.line).size());
}


//
//	TextEditor::Document::findWordStart
//

TextEditor::DocPos TextEditor::Document::findWordStart(DocPos from, bool wordOnly) const {
	const auto& line = at(from.line);
	const auto lineSize = line.size();

	if (from.index == 0 || lineSize == 0) {
		return from;

	} else {
		auto index = from.index;
		const auto firstCharacter = line[index - 1].codepoint;

		if (!wordOnly && CodePoint::isWhiteSpace(firstCharacter)) {
			while (index > 0 && CodePoint::isWhiteSpace(line[index - 1].codepoint)) {
				index--;
			}

		} else if (CodePoint::isWord(firstCharacter)) {
			while (index > 0 && CodePoint::isWord(line[index - 1].codepoint)) {
				index--;
			}

		} else {
			while (!wordOnly && index > 0 && !CodePoint::isWord(line[index - 1].codepoint) && !CodePoint::isWhiteSpace(line[index - 1].codepoint)) {
				index--;
			}
		}

		return DocPos(from.line, index);
	}
}


//
//	TextEditor::Document::findWordEnd
//

TextEditor::DocPos TextEditor::Document::findWordEnd(DocPos from, bool wordOnly) const {
	const auto& line = at(from.line);
	const auto size = line.size();
	auto index = from.index;

	if (index >= size) {
		return from;

	} else {
		const auto firstCharacter = line[index].codepoint;

		if (!wordOnly && CodePoint::isWhiteSpace(firstCharacter)) {
			while (index < size && CodePoint::isWhiteSpace(line[index].codepoint)) {
				index++;
			}

		} else if (CodePoint::isWord(firstCharacter)) {
			while (index < size && CodePoint::isWord(line[index].codepoint)) {
				index++;
			}

		} else {
			while (!wordOnly && index < size && !CodePoint::isWord(line[index].codepoint) && !CodePoint::isWhiteSpace(line[index].codepoint)) {
				index++;
			}
		}
	}

	return DocPos(from.line, index);
}


//
//	TextEditor::Document::findText
//

bool TextEditor::Document::findText(DocPos from, const std::string_view& text, bool caseSensitive, bool wholeWord, DocPos& start, DocPos& end) const {
	// convert input string to vector of codepoints
	std::vector<ImWchar> search;
	const auto endOfText = text.end();
	auto i = text.begin();

	while (i < endOfText) {
		ImWchar character;
		i = CodePoint::read(i, endOfText, &character);
		search.emplace_back(caseSensitive ? character : CodePoint::toLower(character));
	}

	// search document
	const auto startLine = from.line;
	const auto startIndex = from.index;
	auto searchLine = startLine;
	auto searchIndex = startIndex;

	do {
		auto line = searchLine;
		auto index = searchIndex;
		auto lineSize = at(line).size();
		bool done = false;
		size_t j = 0;

		while (!done && j < search.size()) {
			if (search[j] == '\n') {
				if (index == lineSize) {
					if (line == size() - 1) {
						done = true;

					} else {
						line++;
						index = 0;
						lineSize = at(line).size();
						j++;
					}

				} else {
					done = true;
				}

			} else {
				if (index == lineSize) {
					done = true;

				} else {
					auto ch = at(line)[index].codepoint;

					if (!caseSensitive) {
						ch = CodePoint::toLower(ch);
					}

					if (ch == search[j]) {
						index++;
						j++;

					} else {
						done = true;
					}
				}
			}
		}

		if (j == search.size()) {
			start = DocPos(searchLine, searchIndex);
			end = DocPos(line, index);

			if (!wholeWord || isWholeWord(start, end)) {
				return true;
			}
		}

		if (searchIndex == at(searchLine).size()) {
			searchLine = (searchLine == size() - 1) ? 0 : searchLine + 1;
			searchIndex = 0;

		} else {
			searchIndex++;
		}

	} while (searchLine != startLine || searchIndex != startIndex);

	return false;
}


//
//	TextEditor::Document::setUserData
//

void TextEditor::Document::setUserData(size_t line, void* data) {
	if (line < size()) {
		at(static_cast<size_t>(line)).userData = data;
	}
}


//
//	TextEditor::Document::getUserData
//

void* TextEditor::Document::getUserData(size_t line) const {
	if (line < size()) {
		return at(static_cast<size_t>(line)).userData;

	} else {
		return nullptr;
	}
}

//
//	TextEditor::Document::iterateUserData
//

void TextEditor::Document::iterateUserData(std::function<void(size_t line, void* data)> callback) const {
	for (size_t i = 0; i < size(); i++) {
		callback(i, at(i).userData);
	}
}


//
//	isIdentifier
//

static inline bool isIdentifier(TextEditor::Color color) {
	return
		color == TextEditor::Color::identifier ||
		color == TextEditor::Color::knownIdentifier;
}


//
//	TextEditor::Document::iterateIdentifiers
//

void TextEditor::Document::iterateIdentifiers(std::function<void(const std::string&)> callback) const {
	for (size_t i = 0; i < size(); i++) {
		auto p = at(i).begin();
		const auto end = at(i).end();
		char utf8[4];

		while (p < end) {
			if (isIdentifier(p->color)) {
				std::string identifier;

				while (p < end && isIdentifier(p->color)) {
					identifier.append(std::string_view(utf8, CodePoint::write(utf8, p->codepoint)));
					p++;
				}

				callback(identifier);

			} else {
				p++;
			}
		}
	}
}


//
//	TextEditor::Document::isWordStart
//

bool TextEditor::Document::isWordStart(DocPos pos, bool wordOnly) const {
	const auto& line = at(pos.line);

	if (isEndOfLine(pos)) {
		return false;

	} else if (isStartOfLine(pos)) {
		if (line.size() == 0) {
			return false;

		} else {
			return !wordOnly || CodePoint::isWord(line[0].codepoint);
		}

	} else {
		const auto glyph1 = line[pos.index - 1].codepoint;
		const auto glyph2 = line[pos.index].codepoint;

		if (wordOnly) {
			return !CodePoint::isWord(glyph1) && CodePoint::isWord(glyph2);

		} else if (CodePoint::isWord(glyph2)) {
			return !CodePoint::isWord(glyph1);

		} else if (CodePoint::isWhiteSpace(glyph2)) {
			return !CodePoint::isWhiteSpace(glyph1);

		} else {
			return CodePoint::isWord(glyph1) || CodePoint::isWhiteSpace(glyph1);
		}
	}
}


//
//	TextEditor::Document::isWordEnd
//

bool TextEditor::Document::isWordEnd(DocPos pos, bool wordOnly) const {
	const auto& line = at(pos.line);

	if (isStartOfLine(pos)) {
		return false;

	} else if (isEndOfLine(pos)) {
		if (line.size() == 0) {
			return false;

		} else {
			return !wordOnly || CodePoint::isWord(line[line.size() - 1].codepoint);
		}

	} else {
		const auto glyph1 = line[pos.index - 1].codepoint;
		const auto glyph2 = line[pos.index].codepoint;

		if (wordOnly) {
			return CodePoint::isWord(glyph1) && !CodePoint::isWord(glyph2);

		} else if (CodePoint::isWord(glyph1)) {
			return !CodePoint::isWord(glyph2);

		} else if (CodePoint::isWhiteSpace(glyph1)) {
			return !CodePoint::isWhiteSpace(glyph2);

		} else {
			return CodePoint::isWord(glyph2) || CodePoint::isWhiteSpace(glyph2);
		}
	}
}


//
//	TextEditor::Document::isWholeWord
//

bool TextEditor::Document::isWholeWord(DocPos start, DocPos end, bool wordOnly) const {
	if (end <= start || start.line != end.line) {
		return false;

	} else {
		return isWordStart(start, wordOnly) && isWordEnd(end, wordOnly);
	}
}


//
//	TextEditor::Document::getWholeWord
//

TextEditor::DocSelection TextEditor::Document::getWholeWord(DocPos pos, bool wordOnly) const {
	if (wordOnly && !isStartOfLine(pos) && !isEndOfLine(pos) && !CodePoint::isWord(at(pos.line)[pos.index].codepoint)) {
		return DocSelection(pos, pos);

	} else if (isWordStart(pos, wordOnly)) {
		return DocSelection(pos, findWordEnd(pos, wordOnly));

	} else if (isWordEnd(pos, wordOnly)) {
		return DocSelection(findWordStart(pos, wordOnly), pos);

	} else {
		return DocSelection(findWordStart(pos, wordOnly), findWordEnd(pos, wordOnly));
	}
}


//
//	TextEditor::Document::findPreviousNonWhiteSpace
//

TextEditor::DocPos TextEditor::Document::findPreviousNonWhiteSpace(DocPos from, bool includeEndOfLine) const {
	bool done = false;

	while (!done) {
		const auto& line = at(from.line);
		auto index = from.index;

		while (!done && index > 0) {
			if (CodePoint::isWhiteSpace(line[index - 1].codepoint)) {
				index--;

			} else {
				from.index = index;
				done = true;
			}
		}

		if (!done) {
			if (from.line == 0 || !includeEndOfLine) {
				from.index = 0;
				done = true;

			} else {
				from = getLeftBeforeHiddenLines(from);
			}
		}
	}

	return from;
}


//
//	TextEditor::Document::findNextNonWhiteSpace
//

TextEditor::DocPos TextEditor::Document::findNextNonWhiteSpace(DocPos from, bool includeEndOfLine) const {
	bool done = false;

	while (!done) {
		const auto& line = at(from.line);
		auto index = from.index;

		while (!done && index < line.size()) {
			if (CodePoint::isWhiteSpace(line[index].codepoint)) {
				index++;

			} else {
				from.index = index;
				done = true;
			}
		}

		if (!done) {
			if (from.line == size() || !includeEndOfLine) {
				from.index = line.size();
				done = true;

			} else {
				from = getRightAfterHiddenLines(from);
			}
		}
	}

	return from;
}


//
//	TextEditor::Document::normalizePos
//

TextEditor::DocPos TextEditor::Document::normalizePos(DocPos pos) const {
	if (pos.line >= size()) {
		return getBottom();

	} else if (pos.index > at(pos.line).size()) {
		return getEndOfLine(pos);

	} else {
		return pos;
	}
}


//
//	TextEditor::Document::getContext
//

std::string TextEditor::Document::getContext(DocPos pos, std::function<bool(ImWchar)> isContext) const {
	const auto& line = at(pos.line);
	auto start = pos;
	auto end = pos;

	if (end.index < line.size() && isContext(line[end.index].codepoint)) {
		end.index++;

		while (end.index < line.size() && isContext(line[end.index].codepoint)) {
			end.index++;
		}

		while (start.index > 0 && isContext(line[start.index - 1].codepoint)) {
			start.index--;
		}
	}

	return (start == end) ? "" : getSectionText(start, end);
}


//
//	TextEditor::Document::getLeftBeforeHiddenLines
//

TextEditor::DocPos TextEditor::Document::getLeftBeforeHiddenLines(DocPos pos) const {
	if (pos.line == 0) {
		return pos;

	} else {
		auto line = pos.line - 1;

		while (line > 0 && at(line).foldingState == FoldingState::hidden) {
			line--;
		}

		return DocPos(line, at(line).size());
	}
}


//
//	TextEditor::Document::getRightAfterHiddenLines
//

TextEditor::DocPos TextEditor::Document::getRightAfterHiddenLines(DocPos pos) const {
	if (pos.line == size() - 1) {
		return pos;

	} else {
		auto line = pos.line + 1;

		while (line < size() - 1 && at(line).foldingState == FoldingState::hidden) {
			line++;
		}

		return DocPos(line, 0);
	}
}


//
//	TextEditor::Document::appendLine
//

void TextEditor::Document::appendLine() {
	auto& line = emplace_back();

	if (insertor) {
		line.userData = insertor(size() - 1);
	}
}


//
//	TextEditor::Document::insertLine
//

void TextEditor::Document::insertLine(size_t offset) {
	const auto line = insert(begin() + offset, Line());

	if (insertor) {
		line->userData = insertor(offset);
	}
}


//
//	TextEditor::Document::deleteLines
//

void TextEditor::Document::deleteLines(size_t start, size_t end) {
	if (deletor) {
		for (auto i = start; i <= end; i++) {
			deletor(i, at(i).userData);
		}
	}

	erase(begin() + start, begin() + end + 1);
}


//
//	TextEditor::Document::clearDocument
//

void TextEditor::Document::clearDocument() {
	if (deletor) {
		for (size_t i = 0; i < size(); i++) {
			deletor(i, at(i).userData);
		}
	}

	clear();
}


//
//	TextEditor::Document::updateIndents
//

void TextEditor::Document::updateIndents(const Config& config, size_t start, size_t end) {
	for (size_t i = start; i <= end; i++) {
		auto& line = at(i);
		line.indent = 0;
		bool done = false;

		for (size_t j = 0; j < line.size() && !done; j++) {
			if (line[j].codepoint == ' ') {
				line.indent++;

			} else if (line[j].codepoint == '\t') {
				line.indent += config.tabSize - (line.indent % config.tabSize);

			} else {
				done = true;
			}
		}
	}
}


//
//	TextEditor::Document::getSpacesToTab
//

size_t TextEditor::Document::getSpacesToTab(const Config& config, const Line& line, size_t index) {
	size_t columns = 0;

	for (size_t i = 0; i < index; i++) {
		columns += CodePoint::getGlyphWidth(line[i].codepoint);
	}

	return ((columns / config.tabSize) + 1) * config.tabSize - columns;
}


//
//	TextEditor::Transactions::reset
//

void TextEditor::Transactions::reset() {
	clear();
	undoIndex = 0;
	version = 0;
}


//
//	TextEditor::Transactions::add
//

void TextEditor::Transactions::add(std::shared_ptr<Transaction> transaction) {
	resize(undoIndex);
	push_back(transaction);
	undoIndex++;
	version++;

	if (callback) {
		std::vector<Change> changes;

		for (const auto& action : *transaction) {
			auto& change = changes.emplace_back();
			change.insert = action.type == Action::Type::insertText;
			change.start = action.start;
			change.end = action.end;
			change.text = action.text;
		}

		callback(changes);
	}
}


//
//	TextEditor::Transactions::undo
//

void TextEditor::Transactions::undo(const Config& config, Document& document, Cursors& cursors) {
	const auto transaction = at(--undoIndex);

	for (auto action = transaction->rbegin(); action < transaction->rend(); action++) {
		if (action->type == Action::Type::insertText) {
			document.deleteText(config, action->start, action->end);

		} else {
			document.insertText(config, action->start, action->text);
		}
	}

	cursors = transaction->getBeforeState();
	version++;

	if (callback) {
		std::vector<Change> changes;

		for (const auto& action : *transaction) {
			auto& change = changes.emplace_back();
			change.insert = action.type == Action::Type::deleteText;
			change.start = action.start;
			change.end = action.end;
			change.text = action.text;
		}

		callback(changes);
	}
}


//
//	TextEditor::Transactions::redo
//

void TextEditor::Transactions::redo(const Config& config, Document& document, Cursors& cursors) {
	const auto transaction = at(undoIndex++);

	for (auto action = transaction->begin(); action < transaction->end(); action++) {
		if (action->type == Action::Type::insertText) {
			document.insertText(config, action->start, action->text);

		} else {
			document.deleteText(config, action->start, action->end);
		}
	}

	cursors = transaction->getAfterState();
	version++;

	if (callback) {
		std::vector<Change> changes;

		for (const auto& action : *transaction) {
			auto& change = changes.emplace_back();
			change.insert = action.type == Action::Type::insertText;
			change.start = action.start;
			change.end = action.end;
			change.text = action.text;
		}

		callback(changes);
	}
}


//
//	TextEditor::Colorizer::matches
//

bool TextEditor::Colorizer::matches(Line::iterator start, Line::iterator end, const std::string_view& text) {
	// see if text between iterators matches provided UTF-8 string
	auto i = text.begin();

	while (i < text.end()) {
		if (start == end) {
			return false;
		}

		ImWchar codepoint;
		i = CodePoint::read(i, text.end(), &codepoint);

		if ((start++)->codepoint != codepoint) {
			return false;
		}
	}

	return true;
}


//
//	TextEditor::Colorizer::updateLine
//

TextEditor::LineState TextEditor::Colorizer::updateLine(Line& line) {
	// initialize local variables
	auto state = line.state;
	auto nonWhiteSpace = false;
	auto glyph = line.begin();
	const auto end = line.end();
	const Iterator lineEnd(line.data() + line.size());

	// process all glyphs on this line
	while (glyph < end) {
		// start parsing glyphs
		const auto start = glyph;
		const Iterator tokenStart(&*glyph, line.data());

		if (state == LineState::inText) {
			// special handling for preprocessor lines
			if (!nonWhiteSpace && language->preprocess && glyph->codepoint != language->preprocess && !CodePoint::isWhiteSpace(glyph->codepoint)) {
				nonWhiteSpace = true;
			}

			// are we starting a multilevel, multiline comment
			if (language->commentLevelStart) {
				size_t level;
				auto tokenEnd = language->commentLevelStart(tokenStart, lineEnd, level);

				if (tokenEnd != tokenStart) {
					level = std::min(level, maxCommentLevel);
					state = commentLevelToLineState(level);
					const auto size = tokenEnd - tokenStart;
					setColor(glyph, glyph + size, Color::comment);
					glyph += size;
				}
			}

			// are we starting a multilevel, multiline string
			if (glyph == start && language->stringLevelStart) {
				size_t level;
				auto tokenEnd = language->stringLevelStart(tokenStart, lineEnd, level);

				if (tokenEnd != tokenStart) {
					level = std::min(level, maxStringLevel);
					state = stringLevelToLineState(level);
					const auto size = tokenEnd - tokenStart;
					setColor(glyph, glyph + size, Color::string);
					glyph += size;
				}
			}

			if (glyph == start) {
				// mark whitespace characters
				if (CodePoint::isWhiteSpace(glyph->codepoint)) {
					(glyph++)->color = Color::whitespace;

				// are we starting a multiline comment
				} else if (language->commentStart.size() && matches(glyph, end, language->commentStart)) {
					state = LineState::inComment;
					const auto size = language->commentStart.size();
					setColor(glyph, glyph + size, Color::comment);
					glyph += size;

				// handle single line comments
				} else if (language->singleLineComment.size() && matches(glyph, end, language->singleLineComment)) {
					setColor(glyph, end, Color::comment);
					glyph = end;

				} else if (language->singleLineCommentAlt.size() && matches(glyph, end, language->singleLineCommentAlt)) {
					setColor(glyph, end, Color::comment);
					glyph = end;

				// are we starting a special string
				} else if (language->otherStringStart.size() && matches(glyph, end, language->otherStringStart)) {
					state = LineState::inOtherString;
					const auto size = language->otherStringStart.size();
					setColor(glyph, glyph + size, Color::string);
					glyph += size;

				} else if (language->otherStringAltStart.size() && matches(glyph, end, language->otherStringAltStart)) {
					state = LineState::inOtherStringAlt;
					const auto size = language->otherStringAltStart.size();
					setColor(glyph, glyph + size, Color::string);
					glyph += size;

				// are we starting a single quoted string
				} else if (language->hasSingleQuotedStrings && glyph->codepoint == CodePoint::singleQuote) {
					state = LineState::inSingleQuotedString;
					(glyph++)->color = Color::string;

				// are we starting a double quoted string
				} else if (language->hasDoubleQuotedStrings && glyph->codepoint == CodePoint::doubleQuote) {
					state = LineState::inDoubleQuotedString;
					(glyph++)->color = Color::string;

				// is this a preprocessor line
				} else if (language->preprocess && !nonWhiteSpace && glyph->codepoint == language->preprocess) {
					setColor(line.begin(), end, Color::preprocessor);
					glyph = end;
				}
			}

			if (glyph == start) {
				// nothing worked so far so it's time to do some tokenizing
				Color color;
				Iterator tokenEnd;

				// handle custom tokenizer (if we have one)
				if (language->customTokenizer&& (tokenEnd = language->customTokenizer(tokenStart, lineEnd, color)) != tokenStart) {
					const auto size = tokenEnd - tokenStart;
					setColor(glyph, glyph + size, color);
					glyph += size;

				// do we have an identifier
				} else if (language->getIdentifier && (tokenEnd = language->getIdentifier(tokenStart, lineEnd)) != tokenStart) {
					// determine identifier text and color
					const auto size = tokenEnd - tokenStart;
					std::string identifier;
					color = Color::identifier;

					for (auto i = tokenStart; i < tokenEnd; i++) {
						ImWchar codepoint = *i;

						if (!language->caseSensitive) {
							codepoint = CodePoint::toLower(codepoint);
						}

						char utf8[4];
						identifier.append(utf8, CodePoint::write(utf8, codepoint));
					}

					if (language->keywords.find(identifier) != language->keywords.end()) {
						color = Color::keyword;

					} else if (language->declarations.find(identifier) != language->declarations.end()) {
						color = Color::declaration;

					} else if (language->identifiers.find(identifier) != language->identifiers.end()) {
						color = Color::knownIdentifier;
					}

					// colorize identifier and move on
					setColor(glyph, glyph + size, color);
					glyph += size;

				// do we have a number
				} else if (language->getNumber && (tokenEnd = language->getNumber(tokenStart, lineEnd)) != tokenStart) {
					const auto size = tokenEnd - tokenStart;
					setColor(glyph, glyph + size, Color::number);
					glyph += size;

				// is this punctuation
				} else if (language->isPunctuation && language->isPunctuation(glyph->codepoint)) {
					(glyph++)->color = Color::punctuation;

				} else {
					// I guess we don't know what this character is
					(glyph++)->color = Color::text;
				}
			}

		} else if (lineStateInComment(state)) {
			// stay in comment state until we see the end sequence
			auto size = language->commentEnd.size();

			if (size && matches(glyph, end, language->commentEnd)) {
				setColor(glyph, glyph + size, Color::comment);
				glyph += size;

				// nested comments are closed one level at a time
				if (language->nestedComments && state != LineState::inComment) {
					state = static_cast<LineState>(static_cast<int>(state) - 1);

				} else {
					state = LineState::inText;
				}

			} else if (language->nestedComments && matches(glyph, end, language->commentStart)) {
				size = language->commentStart.size();
				setColor(glyph, glyph + size, Color::comment);
				glyph += size;

				if (state != LineState::inCommentLevel7) {
					state = static_cast<LineState>(static_cast<int>(state) + 1);
				}

			} else if (language->commentLevelEnd) {
				size_t level;
				auto tokenEnd = language->commentLevelEnd(tokenStart, lineEnd, level);

				if (tokenEnd != tokenStart) {
					level = std::min(level, maxCommentLevel);

					if (state == commentLevelToLineState(level)) {
						size = tokenEnd - tokenStart;
						setColor(glyph, glyph + size, Color::comment);
						glyph += size;
						state = LineState::inText;
					}
				}
			}

			if (glyph == start) {
				(glyph++)->color = Color::comment;
			}

		} else if (lineStateInStringLevel(state)) {
			// stay in string level until matching closing is detected
			size_t level;
			Iterator tokenEnd = language->stringLevelEnd(tokenStart, lineEnd, level);

			if (tokenEnd != tokenStart) {
				level = std::min(level, maxStringLevel);

				if (state == stringLevelToLineState(level)) {
					const auto size = tokenEnd - tokenStart;
					setColor(glyph, glyph + size, Color::string);
					glyph += size;
					state = LineState::inText;

				} else {
					(glyph++)->color = Color::string;
				}

			} else {
				(glyph++)->color = Color::string;
			}

		} else if (state == LineState::inOtherString) {
			// stay in otherString state until we see the end sequence
			// skip escaped characters
			if (glyph->codepoint == language->stringEscape) {
				(glyph++)->color = Color::string;

				if (glyph < end) {
					(glyph++)->color = Color::string;
				}

			} else if (matches(glyph, end, language->otherStringEnd)) {
				const auto size = language->otherStringEnd.size();
				setColor(glyph, glyph + size, Color::string);
				glyph += size;
				state = LineState::inText;

			} else {
				(glyph++)->color = Color::string;
			}

		} else if (state == LineState::inOtherStringAlt) {
			// stay in otherStringAlt state until we see the end sequence
			// skip escaped characters
			if (glyph->codepoint == language->stringEscape) {
				(glyph++)->color = Color::string;

				if (glyph < end) {
					(glyph++)->color = Color::string;
				}

			} else if (matches(glyph, end, language->otherStringAltEnd)) {
				const auto size = language->otherStringAltEnd.size();
				setColor(glyph, glyph + size, Color::string);
				glyph += size;
				state = LineState::inText;

			} else {
				(glyph++)->color = Color::string;
			}

		} else if (state == LineState::inSingleQuotedString) {
			// stay in single quote state until we see an end
			// skip escaped characters
			if (glyph->codepoint == language->stringEscape) {
				(glyph++)->color = Color::string;

				if (glyph < end) {
					(glyph++)->color = Color::string;
				}

			} else if (glyph->codepoint == CodePoint::singleQuote) {
				(glyph++)->color = Color::string;
				state = LineState::inText;

			} else {
				(glyph++)->color = Color::string;
			}

		} else if (state == LineState::inDoubleQuotedString) {
			// stay in double quote state until we see an end
			// skip escaped characters
			if (glyph->codepoint == language->stringEscape) {
				(glyph++)->color = Color::string;

				if (glyph < end) {
					(glyph++)->color = Color::string;
				}

			} else if (glyph->codepoint == CodePoint::doubleQuote) {
				(glyph++)->color = Color::string;
				state = LineState::inText;

			} else {
				(glyph++)->color = Color::string;
			}
		}
	}

	return state;
}


//
//	TextEditor::Colorizer::update
//

bool TextEditor::Colorizer::update(const Config& config, Document& document) {
	// update all lines on configuration change
	const bool configChanged = language != config.language;

	if (configChanged) {
		language = config.language;

		if (language) {
			for (auto line = document.begin(); line < document.end(); line++) {
				const auto state = updateLine(*line);
				line->needsColorizing = false;
				const auto next = line + 1;

				if (next < document.end()) {
					next->state = state;
				}
			}

		} else {
			for (auto line = document.begin(); line < document.end(); line++) {
				for (auto glyph = line->begin(); glyph < line->end(); glyph++) {
					glyph->color = Color::text;
				}

				line->state = LineState::inText;
				line->needsColorizing = false;
			}
		}

	// update changed lines when document is updated
	} else if (document.isUpdated()) {
		for (auto line = document.begin(); line < document.end(); line++) {
			if (line->needsColorizing) {
				if (language) {
					const auto state = updateLine(*line);
					line->needsColorizing = false;
					const auto next = line + 1;

					if (next < document.end() && next->state != state) {
						next->state = state;
						next->needsColorizing = true;
					}

				} else {
					for (auto glyph = line->begin(); glyph < line->end(); glyph++) {
						glyph->color = Color::text;
					}

					line->state = LineState::inText;
					line->needsColorizing = false;
				}
			}
		}
	}

	// return status
	return configChanged;
}


//
//	TextEditor::Bracketeer::update
//

void TextEditor::Bracketeer::update(const Config& config, Document& document) {
	// see if the configuration changed
	const bool configChanged =
		showMatchingBrackets != config.showMatchingBrackets ||
		language != config.language;

	showMatchingBrackets = config.showMatchingBrackets;
	language = config.language;

	if (configChanged && !showMatchingBrackets) {
		// if configuration changed, clear the bracket pair list
		clear();

		// and reset glyph colors
		for (size_t line = 0; line < document.size(); line++) {
			for (size_t index = 0; index < document[line].size(); index++) {
				auto& glyph = document[line][index];

				if (glyph.color == Color::matchingBracketLevel1 ||
					glyph.color == Color::matchingBracketLevel2 ||
					glyph.color == Color::matchingBracketLevel3 ||
					glyph.color == Color::matchingBracketError) {

					glyph.color = Color::punctuation;
				}
			}
		}

	} else if (showMatchingBrackets && (configChanged || document.isUpdated())) {
		// if configuration or document changed, recalculate bracket pair list
		static const Color bracketColors[] = {
			Color::matchingBracketLevel1,
			Color::matchingBracketLevel2,
			Color::matchingBracketLevel3
		};

		// copy old list so we can see if things have changed
		const auto previous = *this;

		// clear old list
		clear();
		std::vector<size_t> levels;
		size_t level = 0;

		// process all the glyphs
		for (size_t line = 0; line < document.size(); line++) {
			for (size_t index = 0; index < document[line].size(); index++) {
				auto& glyph = document[line][index];

				// handle a "bracket opener" that is not in a comment, string or preprocessor statement
				if (isBracketCandidate(glyph) && CodePoint::isBracketOpener(glyph.codepoint)) {
					// start a new level
					levels.emplace_back(size());
					emplace_back(glyph.codepoint, DocPos(line, index), static_cast<ImWchar>(0), DocPos(0, 0), level, true);
					glyph.color = bracketColors[level % 3];
					level++;

				// handle a "bracket closer" that is not in a comment, string or preprocessor statement
				} else if (isBracketCandidate(glyph) && CodePoint::isBracketCloser(glyph.codepoint)) {
					if (levels.size()) {
						auto& lastBracket = at(levels.back());
						levels.pop_back();
						level--;

						if (lastBracket.startChar == CodePoint::toPairOpener(glyph.codepoint)) {
							// handle matching bracket
							glyph.color = bracketColors[level % 3];
							lastBracket.endChar = glyph.codepoint;
							lastBracket.end = DocPos(line, index);

						} else {
							// no matching bracket, mark brackets as errors
							glyph.color = Color::matchingBracketError;
							document[lastBracket.start.line][lastBracket.start.index].color = Color::matchingBracketError;
							pop_back();
						}

					// this is a closer without an opener
					} else {
						glyph.color = Color::matchingBracketError;
					}
				}
			}
		}

		// handle levels left open and mark them as errors
		if (levels.size()) {
			for (auto i = levels.rbegin(); i < levels.rend(); i++) {
				const auto& start = at(*i).start;
				document[start.line][start.index].color = Color::matchingBracketError;
				erase(begin() + *i);
			}
		}

		// process invisible blocks for languages that are indentation based
		if (language && language->indentationForBlocks) {
			for (size_t i = 0; i < document.size(); i++) {
				if (document[i].size()) {
					const auto currentIndent = document[i].indent;
					auto endLine = i;
					auto done = false;

					for (size_t j = i + 1; j < document.size() && !done; j++) {
						if (document[j].size()) {
							const auto nextIndent = document[j].indent;

							if (nextIndent > currentIndent) {
								endLine = j;

							} else {
								done = true;
							}
						}
					}

					if (endLine > i) {
						emplace_back(DocPos(i, 0), DocPos(endLine, document[endLine].size()));
					}
				}
			}

			// sort visible and invisible blocks by block start
			std::sort(begin(), end(), [](const BracketPair& a, const BracketPair& b) {
				return a.start < b.start;
			});
		}

		updated = previous != *this;
	}
}


//
//	TextEditor::Bracketeer::getEnclosingBrackets
//

TextEditor::Bracketeer::const_iterator TextEditor::Bracketeer::getEnclosingBrackets(DocPos location) const {
	auto brackets = cend();
	bool done = false;

	for (auto i = cbegin(); !done && i < end(); i++) {
		// brackets are sorted so no need to go past specified location
		if (i->isAfter(location)) {
			done = true;
		}

		else if (i->isAround(location)) {
			// this could be what we're looking for
			brackets = i;
		}
	}

	return brackets;
}


//
//	TextEditor::Bracketeer::getEnclosingBrackets
//

TextEditor::Bracketeer::const_iterator TextEditor::Bracketeer::getEnclosingBrackets(DocPos first, DocPos last) const {
	auto brackets = cend();
	bool done = false;

	for (auto i = cbegin(); !done && i < end(); i++) {
		// brackets are sorted so no need to go past specified location
		if (i->isAfter(first)) {
			done = true;
		}

		else if (i->isAround(first) && i->isAround(last)) {
			// this could be what we're looking for
			brackets = i;
		}
	}

	return brackets;
}


//
//	TextEditor::Bracketeer::getInnerBrackets
//

TextEditor::Bracketeer::const_iterator TextEditor::Bracketeer::getInnerBrackets(DocPos first, DocPos last) const {
	auto brackets = cend();
	const auto outer = getEnclosingBrackets(first, last);

	if (outer != end()) {
		bool done = false;

		for (auto i = outer + 1; i < end() && !done; i++) {
			if (i->level <= outer->level) {
				done = true;

			} else if (i->level == outer->level + 1 && i->start > first && i->end < last) {
				brackets = i;
				done = true;
			}
		}
	}

	return brackets;
}


//
//	latchButton
//

static bool latchButton(const char* label, bool* value, const ImVec2& size) {
	auto changed = false;
	const ImVec4* colors = ImGui::GetStyle().Colors;

	if (*value) {
		ImGui::PushStyleColor(ImGuiCol_Button, colors[ImGuiCol_ButtonActive]);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors[ImGuiCol_ButtonActive]);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors[ImGuiCol_TableBorderLight]);

	} else {
		ImGui::PushStyleColor(ImGuiCol_Button, colors[ImGuiCol_TableBorderLight]);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colors[ImGuiCol_TableBorderLight]);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, colors[ImGuiCol_ButtonActive]);
	}

	ImGui::Button(label, size);

	if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
		*value = !*value;
		changed = true;
	}

	ImGui::PopStyleColor(3);
	return changed;
}


//
//	inputString
//

static bool inputString(const char* label, std::string* value, ImGuiInputTextFlags flags=ImGuiInputTextFlags_None) {
	flags |=
		ImGuiInputTextFlags_NoUndoRedo |
		ImGuiInputTextFlags_CallbackResize;

	return ImGui::InputText(label, value->data(), value->capacity() + 1, flags, [](ImGuiInputTextCallbackData* data) {
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
			std::string* value = static_cast<std::string*>(data->UserData);
			value->resize(data->BufTextLen);
			data->Buf = (char*) value->c_str();
		}

		return 0;
	}, value);
}


//
//	TextEditor::renderFindReplace
//

void TextEditor::renderFindReplace() {
	// render find/replace window (if required)
	if (findReplaceVisible) {
		// save current screen position
		const auto currentScreenPosition = ImGui::GetCursorScreenPos();

		// calculate sizes
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 4.0f));
		const auto& style = ImGui::GetStyle();
		const auto fieldWidth = 250.0f * ImGui::GetStyle().FontScaleDpi;

		auto button1Width = ImGui::CalcTextSize(findButtonLabel.c_str()).x + style.ItemSpacing.x * 2.0f;
		auto button2Width = ImGui::CalcTextSize(findAllButtonLabel.c_str()).x + style.ItemSpacing.x * 2.0f;

		auto optionWidth = 0.0f;
		optionWidth = std::max(optionWidth, ImGui::CalcTextSize(findCapsButtonLabel.c_str()).x);
		optionWidth = std::max(optionWidth, ImGui::CalcTextSize(findWholeWordButtonLabel.c_str()).x);
		optionWidth = std::max(optionWidth, ImGui::CalcTextSize(findCloseButtonLabel.c_str()).x);
		optionWidth += style.ItemSpacing.x * 2.0f;

		if (!config.readOnly) {
			button1Width = std::max(button1Width, ImGui::CalcTextSize(replaceButtonLabel.c_str()).x + style.ItemSpacing.x * 2.0f);
			button2Width = std::max(button2Width, ImGui::CalcTextSize(replaceAllButtonLabel.c_str()).x + style.ItemSpacing.x * 2.0f);
		}

		const auto windowHeight =
			style.ChildBorderSize * 2.0f +
			style.WindowPadding.y * 2.0f +
			ImGui::GetFrameHeight() +
			(config.readOnly ? 0.0f : (style.ItemSpacing.y + ImGui::GetFrameHeight()));

		const auto windowWidth =
			style.ChildBorderSize * 2.0f +
			style.WindowPadding.x * 2.0f +
			fieldWidth + style.ItemSpacing.x +
			button1Width + style.ItemSpacing.x +
			button2Width + style.ItemSpacing.x +
			optionWidth * 3.0f + style.ItemSpacing.x * 2.0f;

		// create window
		const auto availableSpace =
			ImGui::GetWindowWidth() -
			(config.showMiniMap ? miniMapWidth : 0.0f) -
			(ImGui::GetCurrentWindow()->ScrollbarY ? ImGui::GetStyle().ScrollbarSize : 0.0f);

		ImGui::SetNextWindowPos(
			ImGui::GetWindowPos() +
			ImVec2(availableSpace - windowWidth - style.ItemSpacing.x, style.ItemSpacing.y * 2.0f));

		ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));
		ImGui::SetNextWindowBgAlpha(0.75f);

		ImGui::BeginChild("find-replace", ImVec2(windowWidth, windowHeight), ImGuiChildFlags_Borders);
		ImGui::SetNextItemWidth(fieldWidth);

		if (focusOnFind) {
			ImGui::SetKeyboardFocusHere();
			focusOnFind = false;

		} else if (findCancelledAutocomplete) {
			ImGui::SetKeyboardFocusHere();
			findCancelledAutocomplete = false;
		}

		if (inputString("###find", &findText, ImGuiInputTextFlags_AutoSelectAll)) {
			if (findText.size()) {
				selectFirstOccurrenceOf(findText, caseSensitiveFind, wholeWordFind);

			} else {
				cursors.clearAll();
			}
		}

		if (ImGui::IsItemDeactivated()) {
			if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
				closeFindReplace();

			} else if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
				focusOnEditor = true;
				focusOnFind = false;
			}
		}

		const bool disableFindButtons = !findText.size();

		if (disableFindButtons) {
			ImGui::BeginDisabled();
		}

		ImGui::SameLine();

		if (ImGui::Button(findButtonLabel.c_str(), ImVec2(button1Width, 0.0f))) {
			find();
		}

		ImGui::SameLine();

		if (ImGui::Button(findAllButtonLabel.c_str(), ImVec2(button2Width, 0.0f))) {
			findAll();
		}

		if (disableFindButtons) {
			ImGui::EndDisabled();
		}

		ImGui::SameLine();

		if (latchButton(findCapsButtonLabel.c_str(), &caseSensitiveFind, ImVec2(optionWidth, 0.0f))) {
			find();
		}

		ImGui::SameLine();

		if (latchButton(findWholeWordButtonLabel.c_str(), &wholeWordFind, ImVec2(optionWidth, 0.0f))) {
			find();
		}

		ImGui::SameLine();

		if (ImGui::Button(findCloseButtonLabel.c_str(), ImVec2(optionWidth, 0.0f))) {
			closeFindReplace();
		}

		if (!config.readOnly) {
			ImGui::SetNextItemWidth(fieldWidth);
			inputString("###replace", &replaceText);
			ImGui::SameLine();

			const bool disableReplaceButtons = !findText.size() || !replaceText.size();

			if (disableReplaceButtons) {
				ImGui::BeginDisabled();
			}

			if (ImGui::Button(replaceButtonLabel.c_str(), ImVec2(button1Width, 0.0f))) {
				replace();
			}

			ImGui::SameLine();

			if (ImGui::Button(replaceAllButtonLabel.c_str(), ImVec2(button2Width, 0.0f))) {
				replaceAll();
			}

			if (disableReplaceButtons) {
				ImGui::EndDisabled();
			}
		}


		if (ImGui::IsWindowFocused() &&
			!disableFindButtons &&
			ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_G)) {

			ImGui::SetWindowFocus(nullptr);
			find();
		}

		ImGui::EndChild();
		ImGui::PopStyleVar();
		ImGui::SetCursorScreenPos(currentScreenPosition);
	}
}


//
//	TextEditor::selectFirstOccurrenceOf
//

void TextEditor::selectFirstOccurrenceOf(const std::string_view& text, bool caseSensitive, bool wholeWord) {
	DocPos start, end;

	if (document.findText(DocPos(0, 0), text, caseSensitive, wholeWord, start, end)) {
		cursors.setCursor(start, end);
		makeCursorVisible();

	} else {
		cursors.clearAdditional(true);
	}
}


//
//	TextEditor::selectNextOccurrenceOf
//

void TextEditor::selectNextOccurrenceOf(const std::string_view& text, bool caseSensitive, bool wholeWord) {
	DocPos start, end;

	if (document.findText(cursors.getCurrent().getSelectionEnd(), text, caseSensitive, wholeWord, start, end)) {
		cursors.setCursor(start, end);
		makeCursorVisible();

	} else {
		cursors.clearAdditional(true);
	}
}


//
//	TextEditor::selectAllOccurrencesOf
//

void TextEditor::selectAllOccurrencesOf(const std::string_view& text, bool caseSensitive, bool wholeWord) {
	DocPos start, end;

	if (document.findText(DocPos(0, 0), text, caseSensitive, wholeWord, start, end)) {
		cursors.setCursor(start, end);
		bool done = false;

		while (!done) {
			DocPos nextStart, nextEnd;
			document.findText(cursors.getCurrent().getSelectionEnd(), text, caseSensitive, wholeWord, nextStart, nextEnd);

			if (nextStart == start && nextEnd == end) {
				done = true;

			} else {
				cursors.addCursor(nextStart, nextEnd);
			}
		}

	} else {
		cursors.clearAdditional(true);
	}
}


//
//	TextEditor::addNextOccurrence
//

void TextEditor::addNextOccurrence(bool wholeWord) {

	const auto cursor = cursors.getCurrent();
	const auto text = document.getSectionText(cursor.getSelectionStart(), cursor.getSelectionEnd());
	DocPos start, end;

	if (document.findText(cursor.getSelectionEnd(), text, true, wholeWord, start, end)) {
		cursors.addCursor(start, end);
	}
}


//
//	TextEditor::selectAllOccurrences
//

void TextEditor::selectAllOccurrences(bool wholeWord) {
	const auto cursor = cursors.getCurrent();
	const auto text = document.getSectionText(cursor.getSelectionStart(), cursor.getSelectionEnd());
	selectAllOccurrencesOf(text, true, wholeWord);
}


//
//	TextEditor::replaceTextInCurrentCursor
//

void TextEditor::replaceTextInCurrentCursor(const std::string_view& text) {
	const auto transaction = startTransaction();

	// first delete old text
	const auto cursor = cursors.getCurrentAsIterator();
	const auto start = cursor->getSelectionStart();
	const auto end = cursor->getSelectionEnd();
	deleteText(transaction, start, end);
	cursors.adjustForDelete(cursor, start, end);

	// now insert new text
	const DocPos newEnd = insertText(transaction, start, text);
	cursor->update(newEnd, false);
	cursors.adjustForInsert(cursor, start, newEnd);

	endTransaction(transaction);
}


//
//	TextEditor::replaceTextInAllCursors
//

void TextEditor::replaceTextInAllCursors(const std::string_view& text) {
	const auto transaction = startTransaction();
	insertTextIntoAllCursors(transaction, text);
	endTransaction(transaction);
}


//
//	TextEditor::replaceSectionText
//

void TextEditor::replaceSectionText(const DocPos& start, const DocPos& end, const std::string_view& text) {
	const auto transaction = startTransaction();
	deleteText(transaction, start, end);
	const auto newEnd = insertText(transaction, start, text);
	cursors.clearAdditional();
	cursors.getMain().update(newEnd, newEnd);
	endTransaction(transaction);
}


//
//	TextEditor::insertSectionText
//

void TextEditor::insertSectionText(DocPos pos, const std::string_view& text) {
	const auto transaction = startTransaction();
	const auto newEnd = insertText(transaction, pos, text);
	cursors.clearAdditional();
	cursors.getMain().update(newEnd, newEnd);
	endTransaction(transaction);
}


//
//	TextEditor::openFindReplace
//

void TextEditor::openFindReplace() {
	// get main cursor location
	const auto cursor = cursors.getMain();

	// see if we have a current selection that's on one line
	if (cursor.hasSelection()) {
		if (cursor.getSelectionStart().line == cursor.getSelectionEnd().line) {
			// use it as the default search
			findText = document.getSectionText(cursor.getSelectionStart(), cursor.getSelectionEnd());
		}

	} else {
		// if cursor is inside a "real" word, use that as the default
		const auto selection = document.getWholeWord(cursor.getSelectionStart(), true);

		if (selection.start != selection.end) {
			findText = document.getSectionText(selection.start, selection.end);
		}
	}

	findReplaceVisible = true;
	focusOnFind = true;
}


//
//	TextEditor::closeFindReplace
//

void TextEditor::closeFindReplace() {
	if (findReplaceVisible) {
		findReplaceVisible = false;
		focusOnEditor = true;
		focusOnFind = false;
	}
}


//
//	TextEditor::find
//

void TextEditor::find() {
	if (findText.size()) {
		selectNextOccurrenceOf(findText, caseSensitiveFind, wholeWordFind);
		focusOnEditor = true;
		focusOnFind = false;
	}
}


//
//	TextEditor::findNext
//

void TextEditor::findNext() {
	if (findText.size()) {
		selectNextOccurrenceOf(findText, caseSensitiveFind, wholeWordFind);
		focusOnEditor = true;
		focusOnFind = false;
	}
}


//
//	TextEditor::findAll
//

void TextEditor::findAll() {
	if (findText.size()) {
		selectAllOccurrencesOf(findText, caseSensitiveFind, wholeWordFind);
		focusOnEditor = true;
		focusOnFind = false;
	}
}


//
//	TextEditor::replace
//

void TextEditor::replace() {
	if (findText.size()) {
		if (!cursors.anyHasSelection()) {
			selectNextOccurrenceOf(findText, caseSensitiveFind, wholeWordFind);
		}

		replaceTextInCurrentCursor(replaceText);
		selectNextOccurrenceOf(findText, caseSensitiveFind, wholeWordFind);
		focusOnEditor = true;
		focusOnFind = false;
	}
}


//
//	TextEditor::replaceAll
//

void TextEditor::replaceAll() {
	if (findText.size()) {
		selectAllOccurrencesOf(findText, caseSensitiveFind, wholeWordFind);
		replaceTextInAllCursors(replaceText);
		focusOnEditor = true;
		focusOnFind = false;
	}
}
//
//	TextEditor::MiniMap::update
//

bool TextEditor::MiniMap::update(const Config& config, const Document& document, const TypeSetter& typeSetter) {
	// update all lines on configuration change
	const bool configChanged = showMiniMap != config.showMiniMap;

	if (configChanged) {
		showMiniMap = config.showMiniMap;
	}

	if (configChanged || typeSetter.isUpdated()) {
		// reset state
		const auto rowCount = typeSetter.getRowCount();
		rows.clear();

		// process all rows
		for (size_t i = 0; i < rowCount; i++) {
			const auto& line = document[typeSetter[i].line];

			if (line.foldingState != FoldingState::hidden) {
				size_t index;
				size_t column;
				size_t endColumn;

				// determine visible part of document line on row
				if (config.wordWrap && line.sections) {
					const auto& section = line.sections->at(typeSetter[i].section);
					index = section.startIndex;
					column = section.indent;
					endColumn = section.columns;

				} else {
					index = 0;
					column = 0;
					endColumn = line.columns;
				}

				// process line
				processLine(line, index, column, endColumn);
			}
		}
	}

	// return status
	return configChanged || typeSetter.isUpdated();
}


//
//	TextEditor::MiniMap::processLine
//

void TextEditor::MiniMap::processLine(const Line& line, size_t index, size_t column, size_t endColumn) {
	auto& row = rows.emplace_back();
	auto start = column;
	auto color = Color::background;

	// process all
	while (column < endColumn) {
		const auto& glyph = line[index++];

		// detect end of section
		if (glyph.color != color) {
			if (column != start) {
				if (color != Color::whitespace) {
					row.sections.emplace_back(start, column, color);
				}

				start = column;
			}

			color = glyph.color;
		}

		// update column number
		column += glyph.columns;
	}

	// handle possible sections at the end of the row
	if (column != start && color != Color::whitespace) {
		row.sections.emplace_back(start, column, color);
	}
}


//
//	line break class types for codepoints
//

enum class LBC : char {
	ai,			// ambiguous (alphabetic or ideograph)
	ak,			// aksara
	al,			// alphabetic
	ap,			// aksara pre-base
	as,			// aksara start
	b2,			// break on either side (but not pair)
	ba,			// break after
	bb,			// break before
	bk,			// break (mandatory)
	cb,			// contingent break
	cj,			// conditional Japanese starter
	cl,			// closing punctuation
	cm,			// combining marks
	cp,			// closing parenthesis
	cr,			// carriage return
	eb,			// emoji base
	em,			// emoji modifier
	ex,			// exclamation/interrogation
	gl,			// glue
	h2,			// hangul lv
	h3,			// hangul lvt
	hh,			// unambiguous hyphen
	hl,			// hebrew letter
	hy,			// hyphen
	id,			// ideographic
	in,			// inseparable characters
	is,			// infix separator
	jl,			// hangul l jamo
	jt,			// hangul t jamo
	jv,			// hangul v jamo
	lf,			// line feed
	nl,			// next line
	ns,			// non-starters
	nu,			// numeric
	op,			// opening punctuation
	po,			// postfix
	pr,			// prefix
	qu,			// ambiguous quotation
	ri,			// regional indicator
	sa,			// complex context dependent (South East Asian)
	sg,			// surrogate (non-tailorable)
	sp,			// space
	sy,			// symbols allowing break after
	vf,			// virama final
	vi,			// virama
	wj,			// word joiner
	xx,			// unknown
	zw,			// zero-width space
	zwj,		// zero width joiner

	sot,		// pseudo class - start of text
	eot,		// pseudo class - end of text
	undefined	// pseudo value for undefined class
};


//
//	Table types
//

template <typename T>
struct LineBreakRange {
	T low;
	T high;
	LBC lbc;
};

using LineBreakRange16 = LineBreakRange<ImWchar16>;
using LineBreakRange32 = LineBreakRange<ImWchar32>;


//
//	lineBreak8
//

static LBC lineBreak8[] = {
	LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::ba, LBC::lf, LBC::bk, LBC::bk,
	LBC::cr, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm,
	LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::sp, LBC::ex, LBC::qu, LBC::al, LBC::pr, LBC::po, LBC::al,
	LBC::qu, LBC::op, LBC::cp, LBC::al, LBC::pr, LBC::is, LBC::hy, LBC::is, LBC::sy, LBC::nu, LBC::nu, LBC::nu, LBC::nu,
	LBC::nu, LBC::nu, LBC::nu, LBC::nu, LBC::nu, LBC::nu, LBC::is, LBC::is, LBC::al, LBC::al, LBC::al, LBC::ex, LBC::al,
	LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al,
	LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al,
	LBC::op, LBC::pr, LBC::cp, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al,
	LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al,
	LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::op, LBC::ba, LBC::cl, LBC::al, LBC::cm, LBC::cm, LBC::cm,
	LBC::cm, LBC::cm, LBC::cm, LBC::nl, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm,
	LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::cm,
	LBC::cm, LBC::cm, LBC::cm, LBC::cm, LBC::gl, LBC::op, LBC::po, LBC::pr, LBC::pr, LBC::pr, LBC::al, LBC::ai, LBC::ai,
	LBC::al, LBC::ai, LBC::qu, LBC::al, LBC::hh, LBC::al, LBC::al, LBC::po, LBC::pr, LBC::ai, LBC::ai, LBC::bb, LBC::al,
	LBC::ai, LBC::is, LBC::ai, LBC::ai, LBC::ai, LBC::qu, LBC::ai, LBC::ai, LBC::ai, LBC::op, LBC::al, LBC::al, LBC::al,
	LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al,
	LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::ai, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al,
	LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al,
	LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al,
	LBC::ai, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al, LBC::al
};


//
//	lineBreak16
//

static LineBreakRange16 lineBreak16[] = {
	{0x0000, 0x0008, LBC::cm}, {0x0009, 0x0009, LBC::ba}, {0x000a, 0x000a, LBC::lf}, {0x000b, 0x000c, LBC::bk},
	{0x000d, 0x000d, LBC::cr}, {0x000e, 0x001f, LBC::cm}, {0x0020, 0x0020, LBC::sp}, {0x0021, 0x0021, LBC::ex},
	{0x0022, 0x0022, LBC::qu}, {0x0023, 0x0023, LBC::al}, {0x0024, 0x0024, LBC::pr}, {0x0025, 0x0025, LBC::po},
	{0x0026, 0x0026, LBC::al}, {0x0027, 0x0027, LBC::qu}, {0x0028, 0x0028, LBC::op}, {0x0029, 0x0029, LBC::cp},
	{0x002a, 0x002a, LBC::al}, {0x002b, 0x002b, LBC::pr}, {0x002c, 0x002c, LBC::is}, {0x002d, 0x002d, LBC::hy},
	{0x002e, 0x002e, LBC::is}, {0x002f, 0x002f, LBC::sy}, {0x0030, 0x0039, LBC::nu}, {0x003a, 0x003b, LBC::is},
	{0x003c, 0x003e, LBC::al}, {0x003f, 0x003f, LBC::ex}, {0x0040, 0x005a, LBC::al}, {0x005b, 0x005b, LBC::op},
	{0x005c, 0x005c, LBC::pr}, {0x005d, 0x005d, LBC::cp}, {0x005e, 0x007a, LBC::al}, {0x007b, 0x007b, LBC::op},
	{0x007c, 0x007c, LBC::ba}, {0x007d, 0x007d, LBC::cl}, {0x007e, 0x007e, LBC::al}, {0x007f, 0x0084, LBC::cm},
	{0x0085, 0x0085, LBC::nl}, {0x0086, 0x009f, LBC::cm}, {0x00a0, 0x00a0, LBC::gl}, {0x00a1, 0x00a1, LBC::op},
	{0x00a2, 0x00a2, LBC::po}, {0x00a3, 0x00a5, LBC::pr}, {0x00a6, 0x00a6, LBC::al}, {0x00a7, 0x00a8, LBC::ai},
	{0x00a9, 0x00a9, LBC::al}, {0x00aa, 0x00aa, LBC::ai}, {0x00ab, 0x00ab, LBC::qu}, {0x00ac, 0x00ac, LBC::al},
	{0x00ad, 0x00ad, LBC::hh}, {0x00ae, 0x00af, LBC::al}, {0x00b0, 0x00b0, LBC::po}, {0x00b1, 0x00b1, LBC::pr},
	{0x00b2, 0x00b3, LBC::ai}, {0x00b4, 0x00b4, LBC::bb}, {0x00b5, 0x00b5, LBC::al}, {0x00b6, 0x00b6, LBC::ai},
	{0x00b7, 0x00b7, LBC::is}, {0x00b8, 0x00ba, LBC::ai}, {0x00bb, 0x00bb, LBC::qu}, {0x00bc, 0x00be, LBC::ai},
	{0x00bf, 0x00bf, LBC::op}, {0x00c0, 0x00d6, LBC::al}, {0x00d7, 0x00d7, LBC::ai}, {0x00d8, 0x00f6, LBC::al},
	{0x00f7, 0x00f7, LBC::ai}, {0x00f8, 0x02c6, LBC::al}, {0x02c7, 0x02c7, LBC::ai}, {0x02c8, 0x02c8, LBC::bb},
	{0x02c9, 0x02cb, LBC::ai}, {0x02cc, 0x02cc, LBC::bb}, {0x02cd, 0x02cd, LBC::ai}, {0x02ce, 0x02cf, LBC::al},
	{0x02d0, 0x02d0, LBC::ai}, {0x02d1, 0x02d7, LBC::al}, {0x02d8, 0x02db, LBC::ai}, {0x02dc, 0x02dc, LBC::al},
	{0x02dd, 0x02dd, LBC::ai}, {0x02de, 0x02de, LBC::al}, {0x02df, 0x02df, LBC::bb}, {0x02e0, 0x02ff, LBC::al},
	{0x0300, 0x035b, LBC::cm}, {0x035c, 0x0362, LBC::gl}, {0x0363, 0x036f, LBC::cm}, {0x0370, 0x0377, LBC::al},
	{0x037a, 0x037d, LBC::al}, {0x037e, 0x037e, LBC::is}, {0x037f, 0x037f, LBC::al}, {0x0384, 0x0386, LBC::al},
	{0x0387, 0x0387, LBC::is}, {0x0388, 0x038a, LBC::al}, {0x038c, 0x038c, LBC::al}, {0x038e, 0x03a1, LBC::al},
	{0x03a3, 0x0482, LBC::al}, {0x0483, 0x0489, LBC::cm}, {0x048a, 0x052f, LBC::al}, {0x0531, 0x0556, LBC::al},
	{0x0558, 0x0588, LBC::al}, {0x0589, 0x0589, LBC::is}, {0x058a, 0x058a, LBC::hh}, {0x058b, 0x058e, LBC::al},
	{0x058f, 0x058f, LBC::pr}, {0x0591, 0x05bd, LBC::cm}, {0x05be, 0x05be, LBC::hh}, {0x05bf, 0x05bf, LBC::cm},
	{0x05c0, 0x05c0, LBC::al}, {0x05c1, 0x05c2, LBC::cm}, {0x05c3, 0x05c3, LBC::al}, {0x05c4, 0x05c5, LBC::cm},
	{0x05c6, 0x05c6, LBC::ex}, {0x05c7, 0x05c9, LBC::cm}, {0x05d0, 0x05ea, LBC::hl}, {0x05ef, 0x05f2, LBC::hl},
	{0x05f3, 0x05f4, LBC::al}, {0x0600, 0x0605, LBC::nu}, {0x0606, 0x0608, LBC::al}, {0x0609, 0x060b, LBC::po},
	{0x060c, 0x060d, LBC::is}, {0x060e, 0x060f, LBC::al}, {0x0610, 0x061a, LBC::cm}, {0x061b, 0x061b, LBC::ex},
	{0x061c, 0x061c, LBC::cm}, {0x061d, 0x061f, LBC::ex}, {0x0620, 0x064a, LBC::al}, {0x064b, 0x065f, LBC::cm},
	{0x0660, 0x0669, LBC::nu}, {0x066a, 0x066a, LBC::po}, {0x066b, 0x066c, LBC::nu}, {0x066d, 0x066f, LBC::al},
	{0x0670, 0x0670, LBC::cm}, {0x0671, 0x06d3, LBC::al}, {0x06d4, 0x06d4, LBC::ex}, {0x06d5, 0x06d5, LBC::al},
	{0x06d6, 0x06dc, LBC::cm}, {0x06dd, 0x06dd, LBC::nu}, {0x06de, 0x06de, LBC::al}, {0x06df, 0x06e4, LBC::cm},
	{0x06e5, 0x06e6, LBC::al}, {0x06e7, 0x06e8, LBC::cm}, {0x06e9, 0x06e9, LBC::al}, {0x06ea, 0x06ed, LBC::cm},
	{0x06ee, 0x06ef, LBC::al}, {0x06f0, 0x06f9, LBC::nu}, {0x06fa, 0x070d, LBC::al}, {0x070f, 0x0710, LBC::al},
	{0x0711, 0x0711, LBC::cm}, {0x0712, 0x072f, LBC::al}, {0x0730, 0x074a, LBC::cm}, {0x074d, 0x07a5, LBC::al},
	{0x07a6, 0x07b0, LBC::cm}, {0x07b1, 0x07b1, LBC::al}, {0x07c0, 0x07c9, LBC::nu}, {0x07ca, 0x07ea, LBC::al},
	{0x07eb, 0x07f3, LBC::cm}, {0x07f4, 0x07f7, LBC::al}, {0x07f8, 0x07f8, LBC::is}, {0x07f9, 0x07f9, LBC::ex},
	{0x07fa, 0x07fa, LBC::al}, {0x07fd, 0x07fd, LBC::cm}, {0x07fe, 0x07ff, LBC::pr}, {0x0800, 0x0815, LBC::al},
	{0x0816, 0x0819, LBC::cm}, {0x081a, 0x081a, LBC::al}, {0x081b, 0x0823, LBC::cm}, {0x0824, 0x0824, LBC::al},
	{0x0825, 0x0827, LBC::cm}, {0x0828, 0x0828, LBC::al}, {0x0829, 0x082d, LBC::cm}, {0x0830, 0x083e, LBC::al},
	{0x0840, 0x0858, LBC::al}, {0x0859, 0x085b, LBC::cm}, {0x085e, 0x085e, LBC::al}, {0x0860, 0x086a, LBC::al},
	{0x0870, 0x088f, LBC::al}, {0x0890, 0x0891, LBC::nu}, {0x0897, 0x089f, LBC::cm}, {0x08a0, 0x08c9, LBC::al},
	{0x08ca, 0x08e1, LBC::cm}, {0x08e2, 0x08e2, LBC::nu}, {0x08e3, 0x0903, LBC::cm}, {0x0904, 0x0939, LBC::al},
	{0x093a, 0x093c, LBC::cm}, {0x093d, 0x093d, LBC::al}, {0x093e, 0x094f, LBC::cm}, {0x0950, 0x0950, LBC::al},
	{0x0951, 0x0957, LBC::cm}, {0x0958, 0x0961, LBC::al}, {0x0962, 0x0963, LBC::cm}, {0x0964, 0x0965, LBC::ba},
	{0x0966, 0x096f, LBC::nu}, {0x0970, 0x0980, LBC::al}, {0x0981, 0x0983, LBC::cm}, {0x0985, 0x098c, LBC::al},
	{0x098f, 0x0990, LBC::al}, {0x0993, 0x09a8, LBC::al}, {0x09aa, 0x09b0, LBC::al}, {0x09b2, 0x09b2, LBC::al},
	{0x09b6, 0x09b9, LBC::al}, {0x09bc, 0x09bc, LBC::cm}, {0x09bd, 0x09bd, LBC::al}, {0x09be, 0x09c4, LBC::cm},
	{0x09c7, 0x09c8, LBC::cm}, {0x09cb, 0x09cd, LBC::cm}, {0x09ce, 0x09ce, LBC::al}, {0x09d7, 0x09d7, LBC::cm},
	{0x09dc, 0x09dd, LBC::al}, {0x09df, 0x09e1, LBC::al}, {0x09e2, 0x09e3, LBC::cm}, {0x09e6, 0x09ef, LBC::nu},
	{0x09f0, 0x09f1, LBC::al}, {0x09f2, 0x09f3, LBC::po}, {0x09f4, 0x09f8, LBC::al}, {0x09f9, 0x09f9, LBC::po},
	{0x09fa, 0x09fa, LBC::al}, {0x09fb, 0x09fb, LBC::pr}, {0x09fc, 0x09fd, LBC::al}, {0x09fe, 0x09fe, LBC::cm},
	{0x0a01, 0x0a03, LBC::cm}, {0x0a05, 0x0a0a, LBC::al}, {0x0a0f, 0x0a10, LBC::al}, {0x0a13, 0x0a28, LBC::al},
	{0x0a2a, 0x0a30, LBC::al}, {0x0a32, 0x0a33, LBC::al}, {0x0a35, 0x0a36, LBC::al}, {0x0a38, 0x0a39, LBC::al},
	{0x0a3c, 0x0a3c, LBC::cm}, {0x0a3e, 0x0a42, LBC::cm}, {0x0a47, 0x0a48, LBC::cm}, {0x0a4b, 0x0a4d, LBC::cm},
	{0x0a51, 0x0a51, LBC::cm}, {0x0a59, 0x0a5c, LBC::al}, {0x0a5e, 0x0a5e, LBC::al}, {0x0a66, 0x0a6f, LBC::nu},
	{0x0a70, 0x0a71, LBC::cm}, {0x0a72, 0x0a74, LBC::al}, {0x0a75, 0x0a75, LBC::cm}, {0x0a76, 0x0a76, LBC::al},
	{0x0a81, 0x0a83, LBC::cm}, {0x0a85, 0x0a8d, LBC::al}, {0x0a8f, 0x0a91, LBC::al}, {0x0a93, 0x0aa8, LBC::al},
	{0x0aaa, 0x0ab0, LBC::al}, {0x0ab2, 0x0ab3, LBC::al}, {0x0ab5, 0x0ab9, LBC::al}, {0x0abc, 0x0abc, LBC::cm},
	{0x0abd, 0x0abd, LBC::al}, {0x0abe, 0x0ac5, LBC::cm}, {0x0ac7, 0x0ac9, LBC::cm}, {0x0acb, 0x0acd, LBC::cm},
	{0x0ad0, 0x0ad0, LBC::al}, {0x0ae0, 0x0ae1, LBC::al}, {0x0ae2, 0x0ae3, LBC::cm}, {0x0ae6, 0x0aef, LBC::nu},
	{0x0af0, 0x0af0, LBC::al}, {0x0af1, 0x0af1, LBC::pr}, {0x0af9, 0x0af9, LBC::al}, {0x0afa, 0x0aff, LBC::cm},
	{0x0b01, 0x0b03, LBC::cm}, {0x0b05, 0x0b0c, LBC::al}, {0x0b0f, 0x0b10, LBC::al}, {0x0b13, 0x0b28, LBC::al},
	{0x0b2a, 0x0b30, LBC::al}, {0x0b32, 0x0b33, LBC::al}, {0x0b35, 0x0b39, LBC::al}, {0x0b3c, 0x0b3c, LBC::cm},
	{0x0b3d, 0x0b3d, LBC::al}, {0x0b3e, 0x0b44, LBC::cm}, {0x0b47, 0x0b48, LBC::cm}, {0x0b4b, 0x0b4d, LBC::cm},
	{0x0b53, 0x0b57, LBC::cm}, {0x0b5c, 0x0b5d, LBC::al}, {0x0b5f, 0x0b61, LBC::al}, {0x0b62, 0x0b63, LBC::cm},
	{0x0b66, 0x0b6f, LBC::nu}, {0x0b70, 0x0b77, LBC::al}, {0x0b82, 0x0b82, LBC::cm}, {0x0b83, 0x0b83, LBC::al},
	{0x0b85, 0x0b8a, LBC::al}, {0x0b8e, 0x0b90, LBC::al}, {0x0b92, 0x0b95, LBC::al}, {0x0b99, 0x0b9a, LBC::al},
	{0x0b9c, 0x0b9c, LBC::al}, {0x0b9e, 0x0b9f, LBC::al}, {0x0ba3, 0x0ba4, LBC::al}, {0x0ba8, 0x0baa, LBC::al},
	{0x0bae, 0x0bb9, LBC::al}, {0x0bbe, 0x0bc2, LBC::cm}, {0x0bc6, 0x0bc8, LBC::cm}, {0x0bca, 0x0bcd, LBC::cm},
	{0x0bd0, 0x0bd0, LBC::al}, {0x0bd7, 0x0bd7, LBC::cm}, {0x0be6, 0x0bef, LBC::nu}, {0x0bf0, 0x0bf8, LBC::al},
	{0x0bf9, 0x0bf9, LBC::pr}, {0x0bfa, 0x0bfa, LBC::al}, {0x0c00, 0x0c04, LBC::cm}, {0x0c05, 0x0c0c, LBC::al},
	{0x0c0e, 0x0c10, LBC::al}, {0x0c12, 0x0c28, LBC::al}, {0x0c2a, 0x0c39, LBC::al}, {0x0c3c, 0x0c3c, LBC::cm},
	{0x0c3d, 0x0c3d, LBC::al}, {0x0c3e, 0x0c44, LBC::cm}, {0x0c46, 0x0c48, LBC::cm}, {0x0c4a, 0x0c4d, LBC::cm},
	{0x0c55, 0x0c56, LBC::cm}, {0x0c58, 0x0c5a, LBC::al}, {0x0c5c, 0x0c5d, LBC::al}, {0x0c60, 0x0c61, LBC::al},
	{0x0c62, 0x0c63, LBC::cm}, {0x0c66, 0x0c6f, LBC::nu}, {0x0c77, 0x0c77, LBC::bb}, {0x0c78, 0x0c80, LBC::al},
	{0x0c81, 0x0c83, LBC::cm}, {0x0c84, 0x0c84, LBC::bb}, {0x0c85, 0x0c8c, LBC::al}, {0x0c8e, 0x0c90, LBC::al},
	{0x0c92, 0x0ca8, LBC::al}, {0x0caa, 0x0cb3, LBC::al}, {0x0cb5, 0x0cb9, LBC::al}, {0x0cbc, 0x0cbc, LBC::cm},
	{0x0cbd, 0x0cbd, LBC::al}, {0x0cbe, 0x0cc4, LBC::cm}, {0x0cc6, 0x0cc8, LBC::cm}, {0x0cca, 0x0ccd, LBC::cm},
	{0x0cd5, 0x0cd6, LBC::cm}, {0x0cdc, 0x0cde, LBC::al}, {0x0ce0, 0x0ce1, LBC::al}, {0x0ce2, 0x0ce3, LBC::cm},
	{0x0ce6, 0x0cef, LBC::nu}, {0x0cf1, 0x0cf2, LBC::al}, {0x0cf3, 0x0cf3, LBC::cm}, {0x0d00, 0x0d03, LBC::cm},
	{0x0d04, 0x0d0c, LBC::al}, {0x0d0e, 0x0d10, LBC::al}, {0x0d12, 0x0d3a, LBC::al}, {0x0d3b, 0x0d3c, LBC::cm},
	{0x0d3d, 0x0d3d, LBC::al}, {0x0d3e, 0x0d44, LBC::cm}, {0x0d46, 0x0d48, LBC::cm}, {0x0d4a, 0x0d4d, LBC::cm},
	{0x0d4e, 0x0d4f, LBC::al}, {0x0d54, 0x0d56, LBC::al}, {0x0d57, 0x0d57, LBC::cm}, {0x0d58, 0x0d61, LBC::al},
	{0x0d62, 0x0d63, LBC::cm}, {0x0d66, 0x0d6f, LBC::nu}, {0x0d70, 0x0d78, LBC::al}, {0x0d79, 0x0d79, LBC::po},
	{0x0d7a, 0x0d7f, LBC::al}, {0x0d81, 0x0d83, LBC::cm}, {0x0d85, 0x0d96, LBC::al}, {0x0d9a, 0x0db1, LBC::al},
	{0x0db3, 0x0dbb, LBC::al}, {0x0dbd, 0x0dbd, LBC::al}, {0x0dc0, 0x0dc6, LBC::al}, {0x0dca, 0x0dca, LBC::cm},
	{0x0dcf, 0x0dd4, LBC::cm}, {0x0dd6, 0x0dd6, LBC::cm}, {0x0dd8, 0x0ddf, LBC::cm}, {0x0de6, 0x0def, LBC::nu},
	{0x0df2, 0x0df3, LBC::cm}, {0x0df4, 0x0df4, LBC::al}, {0x0e01, 0x0e3a, LBC::sa}, {0x0e3f, 0x0e3f, LBC::pr},
	{0x0e40, 0x0e4e, LBC::sa}, {0x0e4f, 0x0e4f, LBC::al}, {0x0e50, 0x0e59, LBC::nu}, {0x0e5a, 0x0e5b, LBC::ba},
	{0x0e81, 0x0e82, LBC::sa}, {0x0e84, 0x0e84, LBC::sa}, {0x0e86, 0x0e8a, LBC::sa}, {0x0e8c, 0x0ea3, LBC::sa},
	{0x0ea5, 0x0ea5, LBC::sa}, {0x0ea7, 0x0ebd, LBC::sa}, {0x0ec0, 0x0ec4, LBC::sa}, {0x0ec6, 0x0ec6, LBC::sa},
	{0x0ec8, 0x0ece, LBC::sa}, {0x0ed0, 0x0ed9, LBC::nu}, {0x0edc, 0x0edf, LBC::sa}, {0x0f00, 0x0f00, LBC::al},
	{0x0f01, 0x0f04, LBC::bb}, {0x0f05, 0x0f05, LBC::al}, {0x0f06, 0x0f07, LBC::bb}, {0x0f08, 0x0f08, LBC::gl},
	{0x0f09, 0x0f0a, LBC::bb}, {0x0f0b, 0x0f0b, LBC::ba}, {0x0f0c, 0x0f0c, LBC::gl}, {0x0f0d, 0x0f11, LBC::ex},
	{0x0f12, 0x0f12, LBC::gl}, {0x0f13, 0x0f13, LBC::al}, {0x0f14, 0x0f14, LBC::ex}, {0x0f15, 0x0f17, LBC::al},
	{0x0f18, 0x0f19, LBC::cm}, {0x0f1a, 0x0f1f, LBC::al}, {0x0f20, 0x0f29, LBC::nu}, {0x0f2a, 0x0f33, LBC::al},
	{0x0f34, 0x0f34, LBC::ba}, {0x0f35, 0x0f35, LBC::cm}, {0x0f36, 0x0f36, LBC::al}, {0x0f37, 0x0f37, LBC::cm},
	{0x0f38, 0x0f38, LBC::al}, {0x0f39, 0x0f39, LBC::cm}, {0x0f3a, 0x0f3a, LBC::op}, {0x0f3b, 0x0f3b, LBC::cl},
	{0x0f3c, 0x0f3c, LBC::op}, {0x0f3d, 0x0f3d, LBC::cl}, {0x0f3e, 0x0f3f, LBC::cm}, {0x0f40, 0x0f47, LBC::al},
	{0x0f49, 0x0f6c, LBC::al}, {0x0f71, 0x0f7e, LBC::cm}, {0x0f7f, 0x0f7f, LBC::ba}, {0x0f80, 0x0f84, LBC::cm},
	{0x0f85, 0x0f85, LBC::ba}, {0x0f86, 0x0f87, LBC::cm}, {0x0f88, 0x0f8c, LBC::al}, {0x0f8d, 0x0f97, LBC::cm},
	{0x0f99, 0x0fbc, LBC::cm}, {0x0fbe, 0x0fbf, LBC::ba}, {0x0fc0, 0x0fc5, LBC::al}, {0x0fc6, 0x0fc6, LBC::cm},
	{0x0fc7, 0x0fcc, LBC::al}, {0x0fce, 0x0fcf, LBC::al}, {0x0fd0, 0x0fd1, LBC::bb}, {0x0fd2, 0x0fd2, LBC::ba},
	{0x0fd3, 0x0fd3, LBC::bb}, {0x0fd4, 0x0fd8, LBC::al}, {0x0fd9, 0x0fda, LBC::gl}, {0x1000, 0x103f, LBC::sa},
	{0x1040, 0x1049, LBC::nu}, {0x104a, 0x104b, LBC::ba}, {0x104c, 0x104f, LBC::al}, {0x1050, 0x108f, LBC::sa},
	{0x1090, 0x1099, LBC::nu}, {0x109a, 0x109f, LBC::sa}, {0x10a0, 0x10c5, LBC::al}, {0x10c7, 0x10c7, LBC::al},
	{0x10cd, 0x10cd, LBC::al}, {0x10d0, 0x10ff, LBC::al}, {0x1100, 0x115f, LBC::jl}, {0x1160, 0x11a7, LBC::jv},
	{0x11a8, 0x11ff, LBC::jt}, {0x1200, 0x1248, LBC::al}, {0x124a, 0x124d, LBC::al}, {0x1250, 0x1256, LBC::al},
	{0x1258, 0x1258, LBC::al}, {0x125a, 0x125d, LBC::al}, {0x1260, 0x1288, LBC::al}, {0x128a, 0x128d, LBC::al},
	{0x1290, 0x12b0, LBC::al}, {0x12b2, 0x12b5, LBC::al}, {0x12b8, 0x12be, LBC::al}, {0x12c0, 0x12c0, LBC::al},
	{0x12c2, 0x12c5, LBC::al}, {0x12c8, 0x12d6, LBC::al}, {0x12d8, 0x1310, LBC::al}, {0x1312, 0x1315, LBC::al},
	{0x1318, 0x135a, LBC::al}, {0x135d, 0x135f, LBC::cm}, {0x1360, 0x1360, LBC::al}, {0x1361, 0x1361, LBC::ba},
	{0x1362, 0x137c, LBC::al}, {0x1380, 0x1399, LBC::al}, {0x13a0, 0x13f5, LBC::al}, {0x13f8, 0x13fd, LBC::al},
	{0x1400, 0x1400, LBC::hh}, {0x1401, 0x167f, LBC::al}, {0x1680, 0x1680, LBC::ba}, {0x1681, 0x169a, LBC::al},
	{0x169b, 0x169b, LBC::op}, {0x169c, 0x169c, LBC::cl}, {0x16a0, 0x16ea, LBC::al}, {0x16eb, 0x16ed, LBC::ba},
	{0x16ee, 0x16f8, LBC::al}, {0x1700, 0x1711, LBC::al}, {0x1712, 0x1715, LBC::cm}, {0x171f, 0x1731, LBC::al},
	{0x1732, 0x1734, LBC::cm}, {0x1735, 0x1736, LBC::ba}, {0x1740, 0x1751, LBC::al}, {0x1752, 0x1753, LBC::cm},
	{0x1760, 0x176c, LBC::al}, {0x176e, 0x1770, LBC::al}, {0x1772, 0x1773, LBC::cm}, {0x1780, 0x17d3, LBC::sa},
	{0x17d4, 0x17d5, LBC::ba}, {0x17d6, 0x17d6, LBC::ns}, {0x17d7, 0x17d7, LBC::sa}, {0x17d8, 0x17d8, LBC::ba},
	{0x17d9, 0x17d9, LBC::al}, {0x17da, 0x17da, LBC::ba}, {0x17db, 0x17db, LBC::pr}, {0x17dc, 0x17dd, LBC::sa},
	{0x17e0, 0x17e9, LBC::nu}, {0x17f0, 0x17f9, LBC::al}, {0x1800, 0x1801, LBC::al}, {0x1802, 0x1803, LBC::ex},
	{0x1804, 0x1805, LBC::ba}, {0x1806, 0x1806, LBC::bb}, {0x1807, 0x1807, LBC::al}, {0x1808, 0x1809, LBC::ex},
	{0x180a, 0x180a, LBC::al}, {0x180b, 0x180d, LBC::cm}, {0x180e, 0x180e, LBC::gl}, {0x180f, 0x180f, LBC::cm},
	{0x1810, 0x1819, LBC::nu}, {0x1820, 0x1878, LBC::al}, {0x1880, 0x1884, LBC::al}, {0x1885, 0x1886, LBC::cm},
	{0x1887, 0x18a8, LBC::al}, {0x18a9, 0x18a9, LBC::cm}, {0x18aa, 0x18aa, LBC::al}, {0x18b0, 0x18f5, LBC::al},
	{0x1900, 0x191e, LBC::al}, {0x1920, 0x192b, LBC::cm}, {0x1930, 0x193b, LBC::cm}, {0x1940, 0x1940, LBC::al},
	{0x1944, 0x1945, LBC::ex}, {0x1946, 0x194f, LBC::nu}, {0x1950, 0x196d, LBC::sa}, {0x1970, 0x1974, LBC::sa},
	{0x1980, 0x19ab, LBC::sa}, {0x19b0, 0x19c9, LBC::sa}, {0x19d0, 0x19da, LBC::nu}, {0x19de, 0x19df, LBC::sa},
	{0x19e0, 0x1a16, LBC::al}, {0x1a17, 0x1a1b, LBC::cm}, {0x1a1e, 0x1a1f, LBC::al}, {0x1a20, 0x1a5e, LBC::sa},
	{0x1a60, 0x1a7c, LBC::sa}, {0x1a7f, 0x1a7f, LBC::cm}, {0x1a80, 0x1a89, LBC::nu}, {0x1a90, 0x1a99, LBC::nu},
	{0x1aa0, 0x1aad, LBC::sa}, {0x1ab0, 0x1aea, LBC::cm}, {0x1aeb, 0x1aeb, LBC::gl}, {0x1aec, 0x1af0, LBC::cm},
	{0x1b00, 0x1b04, LBC::cm}, {0x1b05, 0x1b33, LBC::ak}, {0x1b34, 0x1b43, LBC::cm}, {0x1b44, 0x1b44, LBC::vi},
	{0x1b45, 0x1b4c, LBC::ak}, {0x1b4e, 0x1b4f, LBC::ba}, {0x1b50, 0x1b59, LBC::as}, {0x1b5a, 0x1b5b, LBC::ba},
	{0x1b5c, 0x1b5c, LBC::id}, {0x1b5d, 0x1b60, LBC::ba}, {0x1b61, 0x1b6a, LBC::id}, {0x1b6b, 0x1b73, LBC::cm},
	{0x1b74, 0x1b7c, LBC::id}, {0x1b7d, 0x1b7f, LBC::ba}, {0x1b80, 0x1b82, LBC::cm}, {0x1b83, 0x1ba0, LBC::al},
	{0x1ba1, 0x1bad, LBC::cm}, {0x1bae, 0x1baf, LBC::al}, {0x1bb0, 0x1bb9, LBC::nu}, {0x1bba, 0x1bbf, LBC::al},
	{0x1bc0, 0x1be5, LBC::as}, {0x1be6, 0x1bf1, LBC::cm}, {0x1bf2, 0x1bf3, LBC::vf}, {0x1bfc, 0x1c23, LBC::al},
	{0x1c24, 0x1c37, LBC::cm}, {0x1c3b, 0x1c3f, LBC::ba}, {0x1c40, 0x1c49, LBC::nu}, {0x1c4d, 0x1c4f, LBC::al},
	{0x1c50, 0x1c59, LBC::nu}, {0x1c5a, 0x1c7d, LBC::al}, {0x1c7e, 0x1c7f, LBC::ba}, {0x1c80, 0x1c8a, LBC::al},
	{0x1c90, 0x1cba, LBC::al}, {0x1cbd, 0x1cc7, LBC::al}, {0x1cd0, 0x1cd2, LBC::cm}, {0x1cd3, 0x1cd3, LBC::al},
	{0x1cd4, 0x1ce8, LBC::cm}, {0x1ce9, 0x1cec, LBC::al}, {0x1ced, 0x1ced, LBC::cm}, {0x1cee, 0x1cf3, LBC::al},
	{0x1cf4, 0x1cf4, LBC::cm}, {0x1cf5, 0x1cf6, LBC::al}, {0x1cf7, 0x1cf9, LBC::cm}, {0x1cfa, 0x1cfa, LBC::al},
	{0x1d00, 0x1dbf, LBC::al}, {0x1dc0, 0x1dcc, LBC::cm}, {0x1dcd, 0x1dcd, LBC::gl}, {0x1dce, 0x1dfb, LBC::cm},
	{0x1dfc, 0x1dfc, LBC::gl}, {0x1dfd, 0x1dff, LBC::cm}, {0x1e00, 0x1f15, LBC::al}, {0x1f18, 0x1f1d, LBC::al},
	{0x1f20, 0x1f45, LBC::al}, {0x1f48, 0x1f4d, LBC::al}, {0x1f50, 0x1f57, LBC::al}, {0x1f59, 0x1f59, LBC::al},
	{0x1f5b, 0x1f5b, LBC::al}, {0x1f5d, 0x1f5d, LBC::al}, {0x1f5f, 0x1f7d, LBC::al}, {0x1f80, 0x1fb4, LBC::al},
	{0x1fb6, 0x1fc4, LBC::al}, {0x1fc6, 0x1fd3, LBC::al}, {0x1fd6, 0x1fdb, LBC::al}, {0x1fdd, 0x1fef, LBC::al},
	{0x1ff2, 0x1ff4, LBC::al}, {0x1ff6, 0x1ffc, LBC::al}, {0x1ffd, 0x1ffd, LBC::bb}, {0x1ffe, 0x1ffe, LBC::al},
	{0x2000, 0x2006, LBC::ba}, {0x2007, 0x2007, LBC::gl}, {0x2008, 0x200a, LBC::ba}, {0x200b, 0x200b, LBC::zw},
	{0x200c, 0x200c, LBC::cm}, {0x200d, 0x200d, LBC::zwj}, {0x200e, 0x200f, LBC::cm}, {0x2010, 0x2010, LBC::hh},
	{0x2011, 0x2011, LBC::gl}, {0x2012, 0x2013, LBC::ba}, {0x2014, 0x2014, LBC::b2}, {0x2015, 0x2016, LBC::ai},
	{0x2017, 0x2017, LBC::al}, {0x2018, 0x2019, LBC::qu}, {0x201a, 0x201a, LBC::op}, {0x201b, 0x201d, LBC::qu},
	{0x201e, 0x201e, LBC::op}, {0x201f, 0x201f, LBC::qu}, {0x2020, 0x2021, LBC::ai}, {0x2022, 0x2023, LBC::al},
	{0x2024, 0x2026, LBC::in}, {0x2027, 0x2027, LBC::ba}, {0x2028, 0x2029, LBC::bk}, {0x202a, 0x202e, LBC::cm},
	{0x202f, 0x202f, LBC::gl}, {0x2030, 0x2037, LBC::po}, {0x2038, 0x2038, LBC::al}, {0x2039, 0x203a, LBC::qu},
	{0x203b, 0x203b, LBC::ai}, {0x203c, 0x203d, LBC::ns}, {0x203e, 0x2043, LBC::al}, {0x2044, 0x2044, LBC::is},
	{0x2045, 0x2045, LBC::op}, {0x2046, 0x2046, LBC::cl}, {0x2047, 0x2049, LBC::ns}, {0x204a, 0x2055, LBC::al},
	{0x2056, 0x2056, LBC::ba}, {0x2057, 0x2057, LBC::po}, {0x2058, 0x205b, LBC::ba}, {0x205c, 0x205c, LBC::al},
	{0x205d, 0x205f, LBC::ba}, {0x2060, 0x2060, LBC::wj}, {0x2061, 0x2064, LBC::al}, {0x2066, 0x206f, LBC::cm},
	{0x2070, 0x2071, LBC::al}, {0x2074, 0x2074, LBC::ai}, {0x2075, 0x207c, LBC::al}, {0x207d, 0x207d, LBC::op},
	{0x207e, 0x207e, LBC::cl}, {0x207f, 0x207f, LBC::ai}, {0x2080, 0x2080, LBC::al}, {0x2081, 0x2084, LBC::ai},
	{0x2085, 0x208c, LBC::al}, {0x208d, 0x208d, LBC::op}, {0x208e, 0x208e, LBC::cl}, {0x208f, 0x208f, LBC::bb},
	{0x2090, 0x209f, LBC::al}, {0x20a0, 0x20a6, LBC::pr}, {0x20a7, 0x20a7, LBC::po}, {0x20a8, 0x20b5, LBC::pr},
	{0x20b6, 0x20b6, LBC::po}, {0x20b7, 0x20ba, LBC::pr}, {0x20bb, 0x20bb, LBC::po}, {0x20bc, 0x20bd, LBC::pr},
	{0x20be, 0x20be, LBC::po}, {0x20bf, 0x20bf, LBC::pr}, {0x20c0, 0x20c0, LBC::po}, {0x20c1, 0x20cf, LBC::pr},
	{0x20d0, 0x20f0, LBC::cm}, {0x2100, 0x2102, LBC::al}, {0x2103, 0x2103, LBC::po}, {0x2104, 0x2104, LBC::al},
	{0x2105, 0x2105, LBC::ai}, {0x2106, 0x2108, LBC::al}, {0x2109, 0x2109, LBC::po}, {0x210a, 0x2112, LBC::al},
	{0x2113, 0x2113, LBC::ai}, {0x2114, 0x2115, LBC::al}, {0x2116, 0x2116, LBC::pr}, {0x2117, 0x2120, LBC::al},
	{0x2121, 0x2122, LBC::ai}, {0x2123, 0x212a, LBC::al}, {0x212b, 0x212b, LBC::ai}, {0x212c, 0x214f, LBC::al},
	{0x2150, 0x215e, LBC::ai}, {0x215f, 0x215f, LBC::al}, {0x2160, 0x216b, LBC::ai}, {0x216c, 0x216f, LBC::al},
	{0x2170, 0x2179, LBC::ai}, {0x217a, 0x2188, LBC::al}, {0x2189, 0x2189, LBC::ai}, {0x218a, 0x218b, LBC::al},
	{0x2190, 0x2199, LBC::ai}, {0x219a, 0x21d1, LBC::al}, {0x21d2, 0x21d2, LBC::ai}, {0x21d3, 0x21d3, LBC::al},
	{0x21d4, 0x21d4, LBC::ai}, {0x21d5, 0x21ff, LBC::al}, {0x2200, 0x2200, LBC::ai}, {0x2201, 0x2201, LBC::al},
	{0x2202, 0x2203, LBC::ai}, {0x2204, 0x2206, LBC::al}, {0x2207, 0x2208, LBC::ai}, {0x2209, 0x220a, LBC::al},
	{0x220b, 0x220b, LBC::ai}, {0x220c, 0x220e, LBC::al}, {0x220f, 0x220f, LBC::ai}, {0x2210, 0x2210, LBC::al},
	{0x2211, 0x2211, LBC::ai}, {0x2212, 0x2213, LBC::pr}, {0x2214, 0x2214, LBC::al}, {0x2215, 0x2215, LBC::ai},
	{0x2216, 0x2219, LBC::al}, {0x221a, 0x221a, LBC::ai}, {0x221b, 0x221c, LBC::al}, {0x221d, 0x2220, LBC::ai},
	{0x2221, 0x2222, LBC::al}, {0x2223, 0x2223, LBC::ai}, {0x2224, 0x2224, LBC::al}, {0x2225, 0x2225, LBC::ai},
	{0x2226, 0x2226, LBC::al}, {0x2227, 0x222c, LBC::ai}, {0x222d, 0x222d, LBC::al}, {0x222e, 0x222e, LBC::ai},
	{0x222f, 0x2233, LBC::al}, {0x2234, 0x2237, LBC::ai}, {0x2238, 0x223b, LBC::al}, {0x223c, 0x223d, LBC::ai},
	{0x223e, 0x2247, LBC::al}, {0x2248, 0x2248, LBC::ai}, {0x2249, 0x224b, LBC::al}, {0x224c, 0x224c, LBC::ai},
	{0x224d, 0x2251, LBC::al}, {0x2252, 0x2252, LBC::ai}, {0x2253, 0x225f, LBC::al}, {0x2260, 0x2261, LBC::ai},
	{0x2262, 0x2263, LBC::al}, {0x2264, 0x2267, LBC::ai}, {0x2268, 0x2269, LBC::al}, {0x226a, 0x226b, LBC::ai},
	{0x226c, 0x226d, LBC::al}, {0x226e, 0x226f, LBC::ai}, {0x2270, 0x2281, LBC::al}, {0x2282, 0x2283, LBC::ai},
	{0x2284, 0x2285, LBC::al}, {0x2286, 0x2287, LBC::ai}, {0x2288, 0x2294, LBC::al}, {0x2295, 0x2295, LBC::ai},
	{0x2296, 0x2298, LBC::al}, {0x2299, 0x2299, LBC::ai}, {0x229a, 0x22a4, LBC::al}, {0x22a5, 0x22a5, LBC::ai},
	{0x22a6, 0x22be, LBC::al}, {0x22bf, 0x22bf, LBC::ai}, {0x22c0, 0x22ee, LBC::al}, {0x22ef, 0x22ef, LBC::in},
	{0x22f0, 0x2307, LBC::al}, {0x2308, 0x2308, LBC::op}, {0x2309, 0x2309, LBC::cl}, {0x230a, 0x230a, LBC::op},
	{0x230b, 0x230b, LBC::cl}, {0x230c, 0x2311, LBC::al}, {0x2312, 0x2312, LBC::ai}, {0x2313, 0x2319, LBC::al},
	{0x231a, 0x231b, LBC::id}, {0x231c, 0x2328, LBC::al}, {0x2329, 0x2329, LBC::op}, {0x232a, 0x232a, LBC::cl},
	{0x232b, 0x23ef, LBC::al}, {0x23f0, 0x23f3, LBC::id}, {0x23f4, 0x2429, LBC::al}, {0x2440, 0x244a, LBC::al},
	{0x2460, 0x24fe, LBC::ai}, {0x24ff, 0x24ff, LBC::al}, {0x2500, 0x254b, LBC::ai}, {0x254c, 0x254f, LBC::al},
	{0x2550, 0x2574, LBC::ai}, {0x2575, 0x257f, LBC::al}, {0x2580, 0x258f, LBC::ai}, {0x2590, 0x2591, LBC::al},
	{0x2592, 0x2595, LBC::ai}, {0x2596, 0x259f, LBC::al}, {0x25a0, 0x25a1, LBC::ai}, {0x25a2, 0x25a2, LBC::al},
	{0x25a3, 0x25a9, LBC::ai}, {0x25aa, 0x25b1, LBC::al}, {0x25b2, 0x25b3, LBC::ai}, {0x25b4, 0x25b5, LBC::al},
	{0x25b6, 0x25b7, LBC::ai}, {0x25b8, 0x25bb, LBC::al}, {0x25bc, 0x25bd, LBC::ai}, {0x25be, 0x25bf, LBC::al},
	{0x25c0, 0x25c1, LBC::ai}, {0x25c2, 0x25c5, LBC::al}, {0x25c6, 0x25c8, LBC::ai}, {0x25c9, 0x25ca, LBC::al},
	{0x25cb, 0x25cb, LBC::ai}, {0x25cc, 0x25cd, LBC::al}, {0x25ce, 0x25d1, LBC::ai}, {0x25d2, 0x25e1, LBC::al},
	{0x25e2, 0x25e5, LBC::ai}, {0x25e6, 0x25ee, LBC::al}, {0x25ef, 0x25ef, LBC::ai}, {0x25f0, 0x25ff, LBC::al},
	{0x2600, 0x2603, LBC::id}, {0x2604, 0x2604, LBC::al}, {0x2605, 0x2606, LBC::ai}, {0x2607, 0x2608, LBC::al},
	{0x2609, 0x2609, LBC::ai}, {0x260a, 0x260d, LBC::al}, {0x260e, 0x260f, LBC::ai}, {0x2610, 0x2613, LBC::al},
	{0x2614, 0x2615, LBC::id}, {0x2616, 0x2617, LBC::ai}, {0x2618, 0x2618, LBC::id}, {0x2619, 0x2619, LBC::al},
	{0x261a, 0x261c, LBC::id}, {0x261d, 0x261d, LBC::eb}, {0x261e, 0x261f, LBC::id}, {0x2620, 0x2638, LBC::al},
	{0x2639, 0x263b, LBC::id}, {0x263c, 0x263f, LBC::al}, {0x2640, 0x2640, LBC::ai}, {0x2641, 0x2641, LBC::al},
	{0x2642, 0x2642, LBC::ai}, {0x2643, 0x265f, LBC::al}, {0x2660, 0x2661, LBC::ai}, {0x2662, 0x2662, LBC::al},
	{0x2663, 0x2665, LBC::ai}, {0x2666, 0x2666, LBC::al}, {0x2667, 0x2667, LBC::ai}, {0x2668, 0x2668, LBC::id},
	{0x2669, 0x266a, LBC::ai}, {0x266b, 0x266b, LBC::al}, {0x266c, 0x266d, LBC::ai}, {0x266e, 0x266e, LBC::al},
	{0x266f, 0x266f, LBC::ai}, {0x2670, 0x267e, LBC::al}, {0x267f, 0x267f, LBC::id}, {0x2680, 0x269d, LBC::al},
	{0x269e, 0x269f, LBC::ai}, {0x26a0, 0x26bc, LBC::al}, {0x26bd, 0x26c8, LBC::id}, {0x26c9, 0x26cc, LBC::ai},
	{0x26cd, 0x26cd, LBC::id}, {0x26ce, 0x26ce, LBC::al}, {0x26cf, 0x26d1, LBC::id}, {0x26d2, 0x26d2, LBC::ai},
	{0x26d3, 0x26d4, LBC::id}, {0x26d5, 0x26d7, LBC::ai}, {0x26d8, 0x26d9, LBC::id}, {0x26da, 0x26db, LBC::ai},
	{0x26dc, 0x26dc, LBC::id}, {0x26dd, 0x26de, LBC::ai}, {0x26df, 0x26e1, LBC::id}, {0x26e2, 0x26e2, LBC::al},
	{0x26e3, 0x26e3, LBC::ai}, {0x26e4, 0x26e7, LBC::al}, {0x26e8, 0x26e9, LBC::ai}, {0x26ea, 0x26ea, LBC::id},
	{0x26eb, 0x26f0, LBC::ai}, {0x26f1, 0x26f5, LBC::id}, {0x26f6, 0x26f6, LBC::ai}, {0x26f7, 0x26f8, LBC::id},
	{0x26f9, 0x26f9, LBC::eb}, {0x26fa, 0x26fa, LBC::id}, {0x26fb, 0x26fc, LBC::ai}, {0x26fd, 0x2704, LBC::id},
	{0x2705, 0x2707, LBC::al}, {0x2708, 0x2709, LBC::id}, {0x270a, 0x270d, LBC::eb}, {0x270e, 0x2756, LBC::al},
	{0x2757, 0x2757, LBC::ai}, {0x2758, 0x275a, LBC::al}, {0x275b, 0x2760, LBC::qu}, {0x2761, 0x2761, LBC::al},
	{0x2762, 0x2763, LBC::ex}, {0x2764, 0x2764, LBC::id}, {0x2765, 0x2767, LBC::al}, {0x2768, 0x2768, LBC::op},
	{0x2769, 0x2769, LBC::cl}, {0x276a, 0x276a, LBC::op}, {0x276b, 0x276b, LBC::cl}, {0x276c, 0x276c, LBC::op},
	{0x276d, 0x276d, LBC::cl}, {0x276e, 0x276e, LBC::op}, {0x276f, 0x276f, LBC::cl}, {0x2770, 0x2770, LBC::op},
	{0x2771, 0x2771, LBC::cl}, {0x2772, 0x2772, LBC::op}, {0x2773, 0x2773, LBC::cl}, {0x2774, 0x2774, LBC::op},
	{0x2775, 0x2775, LBC::cl}, {0x2776, 0x2793, LBC::ai}, {0x2794, 0x27c4, LBC::al}, {0x27c5, 0x27c5, LBC::op},
	{0x27c6, 0x27c6, LBC::cl}, {0x27c7, 0x27e5, LBC::al}, {0x27e6, 0x27e6, LBC::op}, {0x27e7, 0x27e7, LBC::cl},
	{0x27e8, 0x27e8, LBC::op}, {0x27e9, 0x27e9, LBC::cl}, {0x27ea, 0x27ea, LBC::op}, {0x27eb, 0x27eb, LBC::cl},
	{0x27ec, 0x27ec, LBC::op}, {0x27ed, 0x27ed, LBC::cl}, {0x27ee, 0x27ee, LBC::op}, {0x27ef, 0x27ef, LBC::cl},
	{0x27f0, 0x27ff, LBC::al}, {0x2800, 0x2800, LBC::ba}, {0x2801, 0x2982, LBC::al}, {0x2983, 0x2983, LBC::op},
	{0x2984, 0x2984, LBC::cl}, {0x2985, 0x2985, LBC::op}, {0x2986, 0x2986, LBC::cl}, {0x2987, 0x2987, LBC::op},
	{0x2988, 0x2988, LBC::cl}, {0x2989, 0x2989, LBC::op}, {0x298a, 0x298a, LBC::cl}, {0x298b, 0x298b, LBC::op},
	{0x298c, 0x298c, LBC::cl}, {0x298d, 0x298d, LBC::op}, {0x298e, 0x298e, LBC::cl}, {0x298f, 0x298f, LBC::op},
	{0x2990, 0x2990, LBC::cl}, {0x2991, 0x2991, LBC::op}, {0x2992, 0x2992, LBC::cl}, {0x2993, 0x2993, LBC::op},
	{0x2994, 0x2994, LBC::cl}, {0x2995, 0x2995, LBC::op}, {0x2996, 0x2996, LBC::cl}, {0x2997, 0x2997, LBC::op},
	{0x2998, 0x2998, LBC::cl}, {0x2999, 0x29d7, LBC::al}, {0x29d8, 0x29d8, LBC::op}, {0x29d9, 0x29d9, LBC::cl},
	{0x29da, 0x29da, LBC::op}, {0x29db, 0x29db, LBC::cl}, {0x29dc, 0x29fb, LBC::al}, {0x29fc, 0x29fc, LBC::op},
	{0x29fd, 0x29fd, LBC::cl}, {0x29fe, 0x2b54, LBC::al}, {0x2b55, 0x2b59, LBC::ai}, {0x2b5a, 0x2b73, LBC::al},
	{0x2b76, 0x2cee, LBC::al}, {0x2cef, 0x2cf1, LBC::cm}, {0x2cf2, 0x2cf3, LBC::al}, {0x2cf9, 0x2cf9, LBC::ex},
	{0x2cfa, 0x2cfc, LBC::ba}, {0x2cfd, 0x2cfd, LBC::al}, {0x2cfe, 0x2cfe, LBC::ex}, {0x2cff, 0x2cff, LBC::ba},
	{0x2d00, 0x2d25, LBC::al}, {0x2d27, 0x2d27, LBC::al}, {0x2d2d, 0x2d2d, LBC::al}, {0x2d30, 0x2d67, LBC::al},
	{0x2d6f, 0x2d6f, LBC::al}, {0x2d70, 0x2d70, LBC::ba}, {0x2d7f, 0x2d7f, LBC::cm}, {0x2d80, 0x2d96, LBC::al},
	{0x2da0, 0x2da6, LBC::al}, {0x2da8, 0x2dae, LBC::al}, {0x2db0, 0x2db6, LBC::al}, {0x2db8, 0x2dbe, LBC::al},
	{0x2dc0, 0x2dc6, LBC::al}, {0x2dc8, 0x2dce, LBC::al}, {0x2dd0, 0x2dd6, LBC::al}, {0x2dd8, 0x2dde, LBC::al},
	{0x2de0, 0x2dff, LBC::cm}, {0x2e00, 0x2e0d, LBC::qu}, {0x2e0e, 0x2e15, LBC::ba}, {0x2e16, 0x2e16, LBC::al},
	{0x2e17, 0x2e17, LBC::hh}, {0x2e18, 0x2e18, LBC::op}, {0x2e19, 0x2e19, LBC::ba}, {0x2e1a, 0x2e1b, LBC::al},
	{0x2e1c, 0x2e1d, LBC::qu}, {0x2e1e, 0x2e1f, LBC::al}, {0x2e20, 0x2e21, LBC::qu}, {0x2e22, 0x2e22, LBC::op},
	{0x2e23, 0x2e23, LBC::cl}, {0x2e24, 0x2e24, LBC::op}, {0x2e25, 0x2e25, LBC::cl}, {0x2e26, 0x2e26, LBC::op},
	{0x2e27, 0x2e27, LBC::cl}, {0x2e28, 0x2e28, LBC::op}, {0x2e29, 0x2e29, LBC::cl}, {0x2e2a, 0x2e2d, LBC::ba},
	{0x2e2e, 0x2e2e, LBC::ex}, {0x2e2f, 0x2e2f, LBC::al}, {0x2e30, 0x2e31, LBC::ba}, {0x2e32, 0x2e32, LBC::al},
	{0x2e33, 0x2e34, LBC::ba}, {0x2e35, 0x2e39, LBC::al}, {0x2e3a, 0x2e3b, LBC::b2}, {0x2e3c, 0x2e3e, LBC::ba},
	{0x2e3f, 0x2e3f, LBC::al}, {0x2e40, 0x2e40, LBC::hh}, {0x2e41, 0x2e41, LBC::ba}, {0x2e42, 0x2e42, LBC::op},
	{0x2e43, 0x2e4a, LBC::ba}, {0x2e4b, 0x2e4b, LBC::al}, {0x2e4c, 0x2e4c, LBC::ba}, {0x2e4d, 0x2e4d, LBC::al},
	{0x2e4e, 0x2e4f, LBC::ba}, {0x2e50, 0x2e52, LBC::al}, {0x2e53, 0x2e54, LBC::ex}, {0x2e55, 0x2e55, LBC::op},
	{0x2e56, 0x2e56, LBC::cp}, {0x2e57, 0x2e57, LBC::op}, {0x2e58, 0x2e58, LBC::cp}, {0x2e59, 0x2e59, LBC::op},
	{0x2e5a, 0x2e5a, LBC::cp}, {0x2e5b, 0x2e5b, LBC::op}, {0x2e5c, 0x2e5c, LBC::cp}, {0x2e5d, 0x2e5d, LBC::hh},
	{0x2e60, 0x2e61, LBC::ex}, {0x2e62, 0x2e62, LBC::op}, {0x2e63, 0x2e63, LBC::cl}, {0x2e80, 0x2e99, LBC::id},
	{0x2e9b, 0x2ef3, LBC::id}, {0x2f00, 0x2fd5, LBC::id}, {0x2ff0, 0x2fff, LBC::id}, {0x3000, 0x3000, LBC::ba},
	{0x3001, 0x3002, LBC::cl}, {0x3003, 0x3004, LBC::id}, {0x3005, 0x3005, LBC::ns}, {0x3006, 0x3007, LBC::id},
	{0x3008, 0x3008, LBC::op}, {0x3009, 0x3009, LBC::cl}, {0x300a, 0x300a, LBC::op}, {0x300b, 0x300b, LBC::cl},
	{0x300c, 0x300c, LBC::op}, {0x300d, 0x300d, LBC::cl}, {0x300e, 0x300e, LBC::op}, {0x300f, 0x300f, LBC::cl},
	{0x3010, 0x3010, LBC::op}, {0x3011, 0x3011, LBC::cl}, {0x3012, 0x3013, LBC::id}, {0x3014, 0x3014, LBC::op},
	{0x3015, 0x3015, LBC::cl}, {0x3016, 0x3016, LBC::op}, {0x3017, 0x3017, LBC::cl}, {0x3018, 0x3018, LBC::op},
	{0x3019, 0x3019, LBC::cl}, {0x301a, 0x301a, LBC::op}, {0x301b, 0x301b, LBC::cl}, {0x301c, 0x301c, LBC::ns},
	{0x301d, 0x301d, LBC::op}, {0x301e, 0x301f, LBC::cl}, {0x3020, 0x3029, LBC::id}, {0x302a, 0x302f, LBC::cm},
	{0x3030, 0x3034, LBC::id}, {0x3035, 0x3035, LBC::cm}, {0x3036, 0x303a, LBC::id}, {0x303b, 0x303c, LBC::ns},
	{0x303d, 0x303f, LBC::id}, {0x3041, 0x3041, LBC::cj}, {0x3042, 0x3042, LBC::id}, {0x3043, 0x3043, LBC::cj},
	{0x3044, 0x3044, LBC::id}, {0x3045, 0x3045, LBC::cj}, {0x3046, 0x3046, LBC::id}, {0x3047, 0x3047, LBC::cj},
	{0x3048, 0x3048, LBC::id}, {0x3049, 0x3049, LBC::cj}, {0x304a, 0x3062, LBC::id}, {0x3063, 0x3063, LBC::cj},
	{0x3064, 0x3082, LBC::id}, {0x3083, 0x3083, LBC::cj}, {0x3084, 0x3084, LBC::id}, {0x3085, 0x3085, LBC::cj},
	{0x3086, 0x3086, LBC::id}, {0x3087, 0x3087, LBC::cj}, {0x3088, 0x308d, LBC::id}, {0x308e, 0x308e, LBC::cj},
	{0x308f, 0x3094, LBC::id}, {0x3095, 0x3096, LBC::cj}, {0x3099, 0x309a, LBC::cm}, {0x309b, 0x309e, LBC::ns},
	{0x309f, 0x309f, LBC::id}, {0x30a0, 0x30a0, LBC::ns}, {0x30a1, 0x30a1, LBC::cj}, {0x30a2, 0x30a2, LBC::id},
	{0x30a3, 0x30a3, LBC::cj}, {0x30a4, 0x30a4, LBC::id}, {0x30a5, 0x30a5, LBC::cj}, {0x30a6, 0x30a6, LBC::id},
	{0x30a7, 0x30a7, LBC::cj}, {0x30a8, 0x30a8, LBC::id}, {0x30a9, 0x30a9, LBC::cj}, {0x30aa, 0x30c2, LBC::id},
	{0x30c3, 0x30c3, LBC::cj}, {0x30c4, 0x30e2, LBC::id}, {0x30e3, 0x30e3, LBC::cj}, {0x30e4, 0x30e4, LBC::id},
	{0x30e5, 0x30e5, LBC::cj}, {0x30e6, 0x30e6, LBC::id}, {0x30e7, 0x30e7, LBC::cj}, {0x30e8, 0x30ed, LBC::id},
	{0x30ee, 0x30ee, LBC::cj}, {0x30ef, 0x30f4, LBC::id}, {0x30f5, 0x30f6, LBC::cj}, {0x30f7, 0x30fa, LBC::id},
	{0x30fb, 0x30fb, LBC::ns}, {0x30fc, 0x30fc, LBC::cj}, {0x30fd, 0x30fe, LBC::ns}, {0x30ff, 0x30ff, LBC::id},
	{0x3105, 0x312f, LBC::id}, {0x3131, 0x318e, LBC::id}, {0x3190, 0x31e5, LBC::id}, {0x31ef, 0x31ef, LBC::id},
	{0x31f0, 0x31ff, LBC::cj}, {0x3200, 0x321e, LBC::id}, {0x3220, 0x3247, LBC::id}, {0x3248, 0x324f, LBC::ai},
	{0x3250, 0x4dbf, LBC::id}, {0x4dc0, 0x4dff, LBC::al}, {0x4e00, 0xa014, LBC::id}, {0xa015, 0xa015, LBC::ns},
	{0xa016, 0xa48c, LBC::id}, {0xa490, 0xa4c6, LBC::id}, {0xa4d0, 0xa4fd, LBC::al}, {0xa4fe, 0xa4ff, LBC::ba},
	{0xa500, 0xa60c, LBC::al}, {0xa60d, 0xa60d, LBC::ba}, {0xa60e, 0xa60e, LBC::ex}, {0xa60f, 0xa60f, LBC::ba},
	{0xa610, 0xa61f, LBC::al}, {0xa620, 0xa629, LBC::nu}, {0xa62a, 0xa62b, LBC::al}, {0xa640, 0xa66e, LBC::al},
	{0xa66f, 0xa672, LBC::cm}, {0xa673, 0xa673, LBC::al}, {0xa674, 0xa67d, LBC::cm}, {0xa67e, 0xa69d, LBC::al},
	{0xa69e, 0xa69f, LBC::cm}, {0xa6a0, 0xa6ef, LBC::al}, {0xa6f0, 0xa6f1, LBC::cm}, {0xa6f2, 0xa6f2, LBC::al},
	{0xa6f3, 0xa6f7, LBC::ba}, {0xa700, 0xa7dd, LBC::al}, {0xa7e2, 0xa7e2, LBC::al}, {0xa7f1, 0xa801, LBC::al},
	{0xa802, 0xa802, LBC::cm}, {0xa803, 0xa805, LBC::al}, {0xa806, 0xa806, LBC::cm}, {0xa807, 0xa80a, LBC::al},
	{0xa80b, 0xa80b, LBC::cm}, {0xa80c, 0xa822, LBC::al}, {0xa823, 0xa827, LBC::cm}, {0xa828, 0xa82b, LBC::al},
	{0xa82c, 0xa82c, LBC::cm}, {0xa830, 0xa837, LBC::al}, {0xa838, 0xa838, LBC::po}, {0xa839, 0xa839, LBC::al},
	{0xa840, 0xa873, LBC::al}, {0xa874, 0xa875, LBC::bb}, {0xa876, 0xa877, LBC::ex}, {0xa880, 0xa881, LBC::cm},
	{0xa882, 0xa8b3, LBC::al}, {0xa8b4, 0xa8c5, LBC::cm}, {0xa8ce, 0xa8cf, LBC::ba}, {0xa8d0, 0xa8d9, LBC::nu},
	{0xa8e0, 0xa8f1, LBC::cm}, {0xa8f2, 0xa8fb, LBC::al}, {0xa8fc, 0xa8fc, LBC::bb}, {0xa8fd, 0xa8fe, LBC::al},
	{0xa8ff, 0xa8ff, LBC::cm}, {0xa900, 0xa909, LBC::nu}, {0xa90a, 0xa925, LBC::al}, {0xa926, 0xa92d, LBC::cm},
	{0xa92e, 0xa92f, LBC::ba}, {0xa930, 0xa946, LBC::al}, {0xa947, 0xa953, LBC::cm}, {0xa95f, 0xa95f, LBC::al},
	{0xa960, 0xa97c, LBC::jl}, {0xa980, 0xa983, LBC::cm}, {0xa984, 0xa9b2, LBC::ak}, {0xa9b3, 0xa9bf, LBC::cm},
	{0xa9c0, 0xa9c0, LBC::vi}, {0xa9c1, 0xa9c6, LBC::id}, {0xa9c7, 0xa9c9, LBC::ba}, {0xa9ca, 0xa9cd, LBC::id},
	{0xa9cf, 0xa9cf, LBC::ba}, {0xa9d0, 0xa9d9, LBC::as}, {0xa9de, 0xa9df, LBC::id}, {0xa9e0, 0xa9ef, LBC::sa},
	{0xa9f0, 0xa9f9, LBC::nu}, {0xa9fa, 0xa9fe, LBC::sa}, {0xaa00, 0xaa28, LBC::as}, {0xaa29, 0xaa36, LBC::cm},
	{0xaa40, 0xaa42, LBC::ba}, {0xaa43, 0xaa43, LBC::cm}, {0xaa44, 0xaa4b, LBC::ba}, {0xaa4c, 0xaa4d, LBC::cm},
	{0xaa50, 0xaa59, LBC::as}, {0xaa5c, 0xaa5c, LBC::id}, {0xaa5d, 0xaa5f, LBC::ba}, {0xaa60, 0xaac2, LBC::sa},
	{0xaadb, 0xaadf, LBC::sa}, {0xaae0, 0xaaea, LBC::al}, {0xaaeb, 0xaaef, LBC::cm}, {0xaaf0, 0xaaf1, LBC::ba},
	{0xaaf2, 0xaaf4, LBC::al}, {0xaaf5, 0xaaf6, LBC::cm}, {0xab01, 0xab06, LBC::al}, {0xab09, 0xab0e, LBC::al},
	{0xab11, 0xab16, LBC::al}, {0xab20, 0xab26, LBC::al}, {0xab28, 0xab2e, LBC::al}, {0xab30, 0xab6d, LBC::al},
	{0xab70, 0xabe2, LBC::al}, {0xabe3, 0xabea, LBC::cm}, {0xabeb, 0xabeb, LBC::ba}, {0xabec, 0xabed, LBC::cm},
	{0xabf0, 0xabf9, LBC::nu}, {0xac00, 0xac00, LBC::h2}, {0xac01, 0xac1b, LBC::h3}, {0xac1c, 0xac1c, LBC::h2},
	{0xac1d, 0xac37, LBC::h3}, {0xac38, 0xac38, LBC::h2}, {0xac39, 0xac53, LBC::h3}, {0xac54, 0xac54, LBC::h2},
	{0xac55, 0xac6f, LBC::h3}, {0xac70, 0xac70, LBC::h2}, {0xac71, 0xac8b, LBC::h3}, {0xac8c, 0xac8c, LBC::h2},
	{0xac8d, 0xaca7, LBC::h3}, {0xaca8, 0xaca8, LBC::h2}, {0xaca9, 0xacc3, LBC::h3}, {0xacc4, 0xacc4, LBC::h2},
	{0xacc5, 0xacdf, LBC::h3}, {0xace0, 0xace0, LBC::h2}, {0xace1, 0xacfb, LBC::h3}, {0xacfc, 0xacfc, LBC::h2},
	{0xacfd, 0xad17, LBC::h3}, {0xad18, 0xad18, LBC::h2}, {0xad19, 0xad33, LBC::h3}, {0xad34, 0xad34, LBC::h2},
	{0xad35, 0xad4f, LBC::h3}, {0xad50, 0xad50, LBC::h2}, {0xad51, 0xad6b, LBC::h3}, {0xad6c, 0xad6c, LBC::h2},
	{0xad6d, 0xad87, LBC::h3}, {0xad88, 0xad88, LBC::h2}, {0xad89, 0xada3, LBC::h3}, {0xada4, 0xada4, LBC::h2},
	{0xada5, 0xadbf, LBC::h3}, {0xadc0, 0xadc0, LBC::h2}, {0xadc1, 0xaddb, LBC::h3}, {0xaddc, 0xaddc, LBC::h2},
	{0xaddd, 0xadf7, LBC::h3}, {0xadf8, 0xadf8, LBC::h2}, {0xadf9, 0xae13, LBC::h3}, {0xae14, 0xae14, LBC::h2},
	{0xae15, 0xae2f, LBC::h3}, {0xae30, 0xae30, LBC::h2}, {0xae31, 0xae4b, LBC::h3}, {0xae4c, 0xae4c, LBC::h2},
	{0xae4d, 0xae67, LBC::h3}, {0xae68, 0xae68, LBC::h2}, {0xae69, 0xae83, LBC::h3}, {0xae84, 0xae84, LBC::h2},
	{0xae85, 0xae9f, LBC::h3}, {0xaea0, 0xaea0, LBC::h2}, {0xaea1, 0xaebb, LBC::h3}, {0xaebc, 0xaebc, LBC::h2},
	{0xaebd, 0xaed7, LBC::h3}, {0xaed8, 0xaed8, LBC::h2}, {0xaed9, 0xaef3, LBC::h3}, {0xaef4, 0xaef4, LBC::h2},
	{0xaef5, 0xaf0f, LBC::h3}, {0xaf10, 0xaf10, LBC::h2}, {0xaf11, 0xaf2b, LBC::h3}, {0xaf2c, 0xaf2c, LBC::h2},
	{0xaf2d, 0xaf47, LBC::h3}, {0xaf48, 0xaf48, LBC::h2}, {0xaf49, 0xaf63, LBC::h3}, {0xaf64, 0xaf64, LBC::h2},
	{0xaf65, 0xaf7f, LBC::h3}, {0xaf80, 0xaf80, LBC::h2}, {0xaf81, 0xaf9b, LBC::h3}, {0xaf9c, 0xaf9c, LBC::h2},
	{0xaf9d, 0xafb7, LBC::h3}, {0xafb8, 0xafb8, LBC::h2}, {0xafb9, 0xafd3, LBC::h3}, {0xafd4, 0xafd4, LBC::h2},
	{0xafd5, 0xafef, LBC::h3}, {0xaff0, 0xaff0, LBC::h2}, {0xaff1, 0xb00b, LBC::h3}, {0xb00c, 0xb00c, LBC::h2},
	{0xb00d, 0xb027, LBC::h3}, {0xb028, 0xb028, LBC::h2}, {0xb029, 0xb043, LBC::h3}, {0xb044, 0xb044, LBC::h2},
	{0xb045, 0xb05f, LBC::h3}, {0xb060, 0xb060, LBC::h2}, {0xb061, 0xb07b, LBC::h3}, {0xb07c, 0xb07c, LBC::h2},
	{0xb07d, 0xb097, LBC::h3}, {0xb098, 0xb098, LBC::h2}, {0xb099, 0xb0b3, LBC::h3}, {0xb0b4, 0xb0b4, LBC::h2},
	{0xb0b5, 0xb0cf, LBC::h3}, {0xb0d0, 0xb0d0, LBC::h2}, {0xb0d1, 0xb0eb, LBC::h3}, {0xb0ec, 0xb0ec, LBC::h2},
	{0xb0ed, 0xb107, LBC::h3}, {0xb108, 0xb108, LBC::h2}, {0xb109, 0xb123, LBC::h3}, {0xb124, 0xb124, LBC::h2},
	{0xb125, 0xb13f, LBC::h3}, {0xb140, 0xb140, LBC::h2}, {0xb141, 0xb15b, LBC::h3}, {0xb15c, 0xb15c, LBC::h2},
	{0xb15d, 0xb177, LBC::h3}, {0xb178, 0xb178, LBC::h2}, {0xb179, 0xb193, LBC::h3}, {0xb194, 0xb194, LBC::h2},
	{0xb195, 0xb1af, LBC::h3}, {0xb1b0, 0xb1b0, LBC::h2}, {0xb1b1, 0xb1cb, LBC::h3}, {0xb1cc, 0xb1cc, LBC::h2},
	{0xb1cd, 0xb1e7, LBC::h3}, {0xb1e8, 0xb1e8, LBC::h2}, {0xb1e9, 0xb203, LBC::h3}, {0xb204, 0xb204, LBC::h2},
	{0xb205, 0xb21f, LBC::h3}, {0xb220, 0xb220, LBC::h2}, {0xb221, 0xb23b, LBC::h3}, {0xb23c, 0xb23c, LBC::h2},
	{0xb23d, 0xb257, LBC::h3}, {0xb258, 0xb258, LBC::h2}, {0xb259, 0xb273, LBC::h3}, {0xb274, 0xb274, LBC::h2},
	{0xb275, 0xb28f, LBC::h3}, {0xb290, 0xb290, LBC::h2}, {0xb291, 0xb2ab, LBC::h3}, {0xb2ac, 0xb2ac, LBC::h2},
	{0xb2ad, 0xb2c7, LBC::h3}, {0xb2c8, 0xb2c8, LBC::h2}, {0xb2c9, 0xb2e3, LBC::h3}, {0xb2e4, 0xb2e4, LBC::h2},
	{0xb2e5, 0xb2ff, LBC::h3}, {0xb300, 0xb300, LBC::h2}, {0xb301, 0xb31b, LBC::h3}, {0xb31c, 0xb31c, LBC::h2},
	{0xb31d, 0xb337, LBC::h3}, {0xb338, 0xb338, LBC::h2}, {0xb339, 0xb353, LBC::h3}, {0xb354, 0xb354, LBC::h2},
	{0xb355, 0xb36f, LBC::h3}, {0xb370, 0xb370, LBC::h2}, {0xb371, 0xb38b, LBC::h3}, {0xb38c, 0xb38c, LBC::h2},
	{0xb38d, 0xb3a7, LBC::h3}, {0xb3a8, 0xb3a8, LBC::h2}, {0xb3a9, 0xb3c3, LBC::h3}, {0xb3c4, 0xb3c4, LBC::h2},
	{0xb3c5, 0xb3df, LBC::h3}, {0xb3e0, 0xb3e0, LBC::h2}, {0xb3e1, 0xb3fb, LBC::h3}, {0xb3fc, 0xb3fc, LBC::h2},
	{0xb3fd, 0xb417, LBC::h3}, {0xb418, 0xb418, LBC::h2}, {0xb419, 0xb433, LBC::h3}, {0xb434, 0xb434, LBC::h2},
	{0xb435, 0xb44f, LBC::h3}, {0xb450, 0xb450, LBC::h2}, {0xb451, 0xb46b, LBC::h3}, {0xb46c, 0xb46c, LBC::h2},
	{0xb46d, 0xb487, LBC::h3}, {0xb488, 0xb488, LBC::h2}, {0xb489, 0xb4a3, LBC::h3}, {0xb4a4, 0xb4a4, LBC::h2},
	{0xb4a5, 0xb4bf, LBC::h3}, {0xb4c0, 0xb4c0, LBC::h2}, {0xb4c1, 0xb4db, LBC::h3}, {0xb4dc, 0xb4dc, LBC::h2},
	{0xb4dd, 0xb4f7, LBC::h3}, {0xb4f8, 0xb4f8, LBC::h2}, {0xb4f9, 0xb513, LBC::h3}, {0xb514, 0xb514, LBC::h2},
	{0xb515, 0xb52f, LBC::h3}, {0xb530, 0xb530, LBC::h2}, {0xb531, 0xb54b, LBC::h3}, {0xb54c, 0xb54c, LBC::h2},
	{0xb54d, 0xb567, LBC::h3}, {0xb568, 0xb568, LBC::h2}, {0xb569, 0xb583, LBC::h3}, {0xb584, 0xb584, LBC::h2},
	{0xb585, 0xb59f, LBC::h3}, {0xb5a0, 0xb5a0, LBC::h2}, {0xb5a1, 0xb5bb, LBC::h3}, {0xb5bc, 0xb5bc, LBC::h2},
	{0xb5bd, 0xb5d7, LBC::h3}, {0xb5d8, 0xb5d8, LBC::h2}, {0xb5d9, 0xb5f3, LBC::h3}, {0xb5f4, 0xb5f4, LBC::h2},
	{0xb5f5, 0xb60f, LBC::h3}, {0xb610, 0xb610, LBC::h2}, {0xb611, 0xb62b, LBC::h3}, {0xb62c, 0xb62c, LBC::h2},
	{0xb62d, 0xb647, LBC::h3}, {0xb648, 0xb648, LBC::h2}, {0xb649, 0xb663, LBC::h3}, {0xb664, 0xb664, LBC::h2},
	{0xb665, 0xb67f, LBC::h3}, {0xb680, 0xb680, LBC::h2}, {0xb681, 0xb69b, LBC::h3}, {0xb69c, 0xb69c, LBC::h2},
	{0xb69d, 0xb6b7, LBC::h3}, {0xb6b8, 0xb6b8, LBC::h2}, {0xb6b9, 0xb6d3, LBC::h3}, {0xb6d4, 0xb6d4, LBC::h2},
	{0xb6d5, 0xb6ef, LBC::h3}, {0xb6f0, 0xb6f0, LBC::h2}, {0xb6f1, 0xb70b, LBC::h3}, {0xb70c, 0xb70c, LBC::h2},
	{0xb70d, 0xb727, LBC::h3}, {0xb728, 0xb728, LBC::h2}, {0xb729, 0xb743, LBC::h3}, {0xb744, 0xb744, LBC::h2},
	{0xb745, 0xb75f, LBC::h3}, {0xb760, 0xb760, LBC::h2}, {0xb761, 0xb77b, LBC::h3}, {0xb77c, 0xb77c, LBC::h2},
	{0xb77d, 0xb797, LBC::h3}, {0xb798, 0xb798, LBC::h2}, {0xb799, 0xb7b3, LBC::h3}, {0xb7b4, 0xb7b4, LBC::h2},
	{0xb7b5, 0xb7cf, LBC::h3}, {0xb7d0, 0xb7d0, LBC::h2}, {0xb7d1, 0xb7eb, LBC::h3}, {0xb7ec, 0xb7ec, LBC::h2},
	{0xb7ed, 0xb807, LBC::h3}, {0xb808, 0xb808, LBC::h2}, {0xb809, 0xb823, LBC::h3}, {0xb824, 0xb824, LBC::h2},
	{0xb825, 0xb83f, LBC::h3}, {0xb840, 0xb840, LBC::h2}, {0xb841, 0xb85b, LBC::h3}, {0xb85c, 0xb85c, LBC::h2},
	{0xb85d, 0xb877, LBC::h3}, {0xb878, 0xb878, LBC::h2}, {0xb879, 0xb893, LBC::h3}, {0xb894, 0xb894, LBC::h2},
	{0xb895, 0xb8af, LBC::h3}, {0xb8b0, 0xb8b0, LBC::h2}, {0xb8b1, 0xb8cb, LBC::h3}, {0xb8cc, 0xb8cc, LBC::h2},
	{0xb8cd, 0xb8e7, LBC::h3}, {0xb8e8, 0xb8e8, LBC::h2}, {0xb8e9, 0xb903, LBC::h3}, {0xb904, 0xb904, LBC::h2},
	{0xb905, 0xb91f, LBC::h3}, {0xb920, 0xb920, LBC::h2}, {0xb921, 0xb93b, LBC::h3}, {0xb93c, 0xb93c, LBC::h2},
	{0xb93d, 0xb957, LBC::h3}, {0xb958, 0xb958, LBC::h2}, {0xb959, 0xb973, LBC::h3}, {0xb974, 0xb974, LBC::h2},
	{0xb975, 0xb98f, LBC::h3}, {0xb990, 0xb990, LBC::h2}, {0xb991, 0xb9ab, LBC::h3}, {0xb9ac, 0xb9ac, LBC::h2},
	{0xb9ad, 0xb9c7, LBC::h3}, {0xb9c8, 0xb9c8, LBC::h2}, {0xb9c9, 0xb9e3, LBC::h3}, {0xb9e4, 0xb9e4, LBC::h2},
	{0xb9e5, 0xb9ff, LBC::h3}, {0xba00, 0xba00, LBC::h2}, {0xba01, 0xba1b, LBC::h3}, {0xba1c, 0xba1c, LBC::h2},
	{0xba1d, 0xba37, LBC::h3}, {0xba38, 0xba38, LBC::h2}, {0xba39, 0xba53, LBC::h3}, {0xba54, 0xba54, LBC::h2},
	{0xba55, 0xba6f, LBC::h3}, {0xba70, 0xba70, LBC::h2}, {0xba71, 0xba8b, LBC::h3}, {0xba8c, 0xba8c, LBC::h2},
	{0xba8d, 0xbaa7, LBC::h3}, {0xbaa8, 0xbaa8, LBC::h2}, {0xbaa9, 0xbac3, LBC::h3}, {0xbac4, 0xbac4, LBC::h2},
	{0xbac5, 0xbadf, LBC::h3}, {0xbae0, 0xbae0, LBC::h2}, {0xbae1, 0xbafb, LBC::h3}, {0xbafc, 0xbafc, LBC::h2},
	{0xbafd, 0xbb17, LBC::h3}, {0xbb18, 0xbb18, LBC::h2}, {0xbb19, 0xbb33, LBC::h3}, {0xbb34, 0xbb34, LBC::h2},
	{0xbb35, 0xbb4f, LBC::h3}, {0xbb50, 0xbb50, LBC::h2}, {0xbb51, 0xbb6b, LBC::h3}, {0xbb6c, 0xbb6c, LBC::h2},
	{0xbb6d, 0xbb87, LBC::h3}, {0xbb88, 0xbb88, LBC::h2}, {0xbb89, 0xbba3, LBC::h3}, {0xbba4, 0xbba4, LBC::h2},
	{0xbba5, 0xbbbf, LBC::h3}, {0xbbc0, 0xbbc0, LBC::h2}, {0xbbc1, 0xbbdb, LBC::h3}, {0xbbdc, 0xbbdc, LBC::h2},
	{0xbbdd, 0xbbf7, LBC::h3}, {0xbbf8, 0xbbf8, LBC::h2}, {0xbbf9, 0xbc13, LBC::h3}, {0xbc14, 0xbc14, LBC::h2},
	{0xbc15, 0xbc2f, LBC::h3}, {0xbc30, 0xbc30, LBC::h2}, {0xbc31, 0xbc4b, LBC::h3}, {0xbc4c, 0xbc4c, LBC::h2},
	{0xbc4d, 0xbc67, LBC::h3}, {0xbc68, 0xbc68, LBC::h2}, {0xbc69, 0xbc83, LBC::h3}, {0xbc84, 0xbc84, LBC::h2},
	{0xbc85, 0xbc9f, LBC::h3}, {0xbca0, 0xbca0, LBC::h2}, {0xbca1, 0xbcbb, LBC::h3}, {0xbcbc, 0xbcbc, LBC::h2},
	{0xbcbd, 0xbcd7, LBC::h3}, {0xbcd8, 0xbcd8, LBC::h2}, {0xbcd9, 0xbcf3, LBC::h3}, {0xbcf4, 0xbcf4, LBC::h2},
	{0xbcf5, 0xbd0f, LBC::h3}, {0xbd10, 0xbd10, LBC::h2}, {0xbd11, 0xbd2b, LBC::h3}, {0xbd2c, 0xbd2c, LBC::h2},
	{0xbd2d, 0xbd47, LBC::h3}, {0xbd48, 0xbd48, LBC::h2}, {0xbd49, 0xbd63, LBC::h3}, {0xbd64, 0xbd64, LBC::h2},
	{0xbd65, 0xbd7f, LBC::h3}, {0xbd80, 0xbd80, LBC::h2}, {0xbd81, 0xbd9b, LBC::h3}, {0xbd9c, 0xbd9c, LBC::h2},
	{0xbd9d, 0xbdb7, LBC::h3}, {0xbdb8, 0xbdb8, LBC::h2}, {0xbdb9, 0xbdd3, LBC::h3}, {0xbdd4, 0xbdd4, LBC::h2},
	{0xbdd5, 0xbdef, LBC::h3}, {0xbdf0, 0xbdf0, LBC::h2}, {0xbdf1, 0xbe0b, LBC::h3}, {0xbe0c, 0xbe0c, LBC::h2},
	{0xbe0d, 0xbe27, LBC::h3}, {0xbe28, 0xbe28, LBC::h2}, {0xbe29, 0xbe43, LBC::h3}, {0xbe44, 0xbe44, LBC::h2},
	{0xbe45, 0xbe5f, LBC::h3}, {0xbe60, 0xbe60, LBC::h2}, {0xbe61, 0xbe7b, LBC::h3}, {0xbe7c, 0xbe7c, LBC::h2},
	{0xbe7d, 0xbe97, LBC::h3}, {0xbe98, 0xbe98, LBC::h2}, {0xbe99, 0xbeb3, LBC::h3}, {0xbeb4, 0xbeb4, LBC::h2},
	{0xbeb5, 0xbecf, LBC::h3}, {0xbed0, 0xbed0, LBC::h2}, {0xbed1, 0xbeeb, LBC::h3}, {0xbeec, 0xbeec, LBC::h2},
	{0xbeed, 0xbf07, LBC::h3}, {0xbf08, 0xbf08, LBC::h2}, {0xbf09, 0xbf23, LBC::h3}, {0xbf24, 0xbf24, LBC::h2},
	{0xbf25, 0xbf3f, LBC::h3}, {0xbf40, 0xbf40, LBC::h2}, {0xbf41, 0xbf5b, LBC::h3}, {0xbf5c, 0xbf5c, LBC::h2},
	{0xbf5d, 0xbf77, LBC::h3}, {0xbf78, 0xbf78, LBC::h2}, {0xbf79, 0xbf93, LBC::h3}, {0xbf94, 0xbf94, LBC::h2},
	{0xbf95, 0xbfaf, LBC::h3}, {0xbfb0, 0xbfb0, LBC::h2}, {0xbfb1, 0xbfcb, LBC::h3}, {0xbfcc, 0xbfcc, LBC::h2},
	{0xbfcd, 0xbfe7, LBC::h3}, {0xbfe8, 0xbfe8, LBC::h2}, {0xbfe9, 0xc003, LBC::h3}, {0xc004, 0xc004, LBC::h2},
	{0xc005, 0xc01f, LBC::h3}, {0xc020, 0xc020, LBC::h2}, {0xc021, 0xc03b, LBC::h3}, {0xc03c, 0xc03c, LBC::h2},
	{0xc03d, 0xc057, LBC::h3}, {0xc058, 0xc058, LBC::h2}, {0xc059, 0xc073, LBC::h3}, {0xc074, 0xc074, LBC::h2},
	{0xc075, 0xc08f, LBC::h3}, {0xc090, 0xc090, LBC::h2}, {0xc091, 0xc0ab, LBC::h3}, {0xc0ac, 0xc0ac, LBC::h2},
	{0xc0ad, 0xc0c7, LBC::h3}, {0xc0c8, 0xc0c8, LBC::h2}, {0xc0c9, 0xc0e3, LBC::h3}, {0xc0e4, 0xc0e4, LBC::h2},
	{0xc0e5, 0xc0ff, LBC::h3}, {0xc100, 0xc100, LBC::h2}, {0xc101, 0xc11b, LBC::h3}, {0xc11c, 0xc11c, LBC::h2},
	{0xc11d, 0xc137, LBC::h3}, {0xc138, 0xc138, LBC::h2}, {0xc139, 0xc153, LBC::h3}, {0xc154, 0xc154, LBC::h2},
	{0xc155, 0xc16f, LBC::h3}, {0xc170, 0xc170, LBC::h2}, {0xc171, 0xc18b, LBC::h3}, {0xc18c, 0xc18c, LBC::h2},
	{0xc18d, 0xc1a7, LBC::h3}, {0xc1a8, 0xc1a8, LBC::h2}, {0xc1a9, 0xc1c3, LBC::h3}, {0xc1c4, 0xc1c4, LBC::h2},
	{0xc1c5, 0xc1df, LBC::h3}, {0xc1e0, 0xc1e0, LBC::h2}, {0xc1e1, 0xc1fb, LBC::h3}, {0xc1fc, 0xc1fc, LBC::h2},
	{0xc1fd, 0xc217, LBC::h3}, {0xc218, 0xc218, LBC::h2}, {0xc219, 0xc233, LBC::h3}, {0xc234, 0xc234, LBC::h2},
	{0xc235, 0xc24f, LBC::h3}, {0xc250, 0xc250, LBC::h2}, {0xc251, 0xc26b, LBC::h3}, {0xc26c, 0xc26c, LBC::h2},
	{0xc26d, 0xc287, LBC::h3}, {0xc288, 0xc288, LBC::h2}, {0xc289, 0xc2a3, LBC::h3}, {0xc2a4, 0xc2a4, LBC::h2},
	{0xc2a5, 0xc2bf, LBC::h3}, {0xc2c0, 0xc2c0, LBC::h2}, {0xc2c1, 0xc2db, LBC::h3}, {0xc2dc, 0xc2dc, LBC::h2},
	{0xc2dd, 0xc2f7, LBC::h3}, {0xc2f8, 0xc2f8, LBC::h2}, {0xc2f9, 0xc313, LBC::h3}, {0xc314, 0xc314, LBC::h2},
	{0xc315, 0xc32f, LBC::h3}, {0xc330, 0xc330, LBC::h2}, {0xc331, 0xc34b, LBC::h3}, {0xc34c, 0xc34c, LBC::h2},
	{0xc34d, 0xc367, LBC::h3}, {0xc368, 0xc368, LBC::h2}, {0xc369, 0xc383, LBC::h3}, {0xc384, 0xc384, LBC::h2},
	{0xc385, 0xc39f, LBC::h3}, {0xc3a0, 0xc3a0, LBC::h2}, {0xc3a1, 0xc3bb, LBC::h3}, {0xc3bc, 0xc3bc, LBC::h2},
	{0xc3bd, 0xc3d7, LBC::h3}, {0xc3d8, 0xc3d8, LBC::h2}, {0xc3d9, 0xc3f3, LBC::h3}, {0xc3f4, 0xc3f4, LBC::h2},
	{0xc3f5, 0xc40f, LBC::h3}, {0xc410, 0xc410, LBC::h2}, {0xc411, 0xc42b, LBC::h3}, {0xc42c, 0xc42c, LBC::h2},
	{0xc42d, 0xc447, LBC::h3}, {0xc448, 0xc448, LBC::h2}, {0xc449, 0xc463, LBC::h3}, {0xc464, 0xc464, LBC::h2},
	{0xc465, 0xc47f, LBC::h3}, {0xc480, 0xc480, LBC::h2}, {0xc481, 0xc49b, LBC::h3}, {0xc49c, 0xc49c, LBC::h2},
	{0xc49d, 0xc4b7, LBC::h3}, {0xc4b8, 0xc4b8, LBC::h2}, {0xc4b9, 0xc4d3, LBC::h3}, {0xc4d4, 0xc4d4, LBC::h2},
	{0xc4d5, 0xc4ef, LBC::h3}, {0xc4f0, 0xc4f0, LBC::h2}, {0xc4f1, 0xc50b, LBC::h3}, {0xc50c, 0xc50c, LBC::h2},
	{0xc50d, 0xc527, LBC::h3}, {0xc528, 0xc528, LBC::h2}, {0xc529, 0xc543, LBC::h3}, {0xc544, 0xc544, LBC::h2},
	{0xc545, 0xc55f, LBC::h3}, {0xc560, 0xc560, LBC::h2}, {0xc561, 0xc57b, LBC::h3}, {0xc57c, 0xc57c, LBC::h2},
	{0xc57d, 0xc597, LBC::h3}, {0xc598, 0xc598, LBC::h2}, {0xc599, 0xc5b3, LBC::h3}, {0xc5b4, 0xc5b4, LBC::h2},
	{0xc5b5, 0xc5cf, LBC::h3}, {0xc5d0, 0xc5d0, LBC::h2}, {0xc5d1, 0xc5eb, LBC::h3}, {0xc5ec, 0xc5ec, LBC::h2},
	{0xc5ed, 0xc607, LBC::h3}, {0xc608, 0xc608, LBC::h2}, {0xc609, 0xc623, LBC::h3}, {0xc624, 0xc624, LBC::h2},
	{0xc625, 0xc63f, LBC::h3}, {0xc640, 0xc640, LBC::h2}, {0xc641, 0xc65b, LBC::h3}, {0xc65c, 0xc65c, LBC::h2},
	{0xc65d, 0xc677, LBC::h3}, {0xc678, 0xc678, LBC::h2}, {0xc679, 0xc693, LBC::h3}, {0xc694, 0xc694, LBC::h2},
	{0xc695, 0xc6af, LBC::h3}, {0xc6b0, 0xc6b0, LBC::h2}, {0xc6b1, 0xc6cb, LBC::h3}, {0xc6cc, 0xc6cc, LBC::h2},
	{0xc6cd, 0xc6e7, LBC::h3}, {0xc6e8, 0xc6e8, LBC::h2}, {0xc6e9, 0xc703, LBC::h3}, {0xc704, 0xc704, LBC::h2},
	{0xc705, 0xc71f, LBC::h3}, {0xc720, 0xc720, LBC::h2}, {0xc721, 0xc73b, LBC::h3}, {0xc73c, 0xc73c, LBC::h2},
	{0xc73d, 0xc757, LBC::h3}, {0xc758, 0xc758, LBC::h2}, {0xc759, 0xc773, LBC::h3}, {0xc774, 0xc774, LBC::h2},
	{0xc775, 0xc78f, LBC::h3}, {0xc790, 0xc790, LBC::h2}, {0xc791, 0xc7ab, LBC::h3}, {0xc7ac, 0xc7ac, LBC::h2},
	{0xc7ad, 0xc7c7, LBC::h3}, {0xc7c8, 0xc7c8, LBC::h2}, {0xc7c9, 0xc7e3, LBC::h3}, {0xc7e4, 0xc7e4, LBC::h2},
	{0xc7e5, 0xc7ff, LBC::h3}, {0xc800, 0xc800, LBC::h2}, {0xc801, 0xc81b, LBC::h3}, {0xc81c, 0xc81c, LBC::h2},
	{0xc81d, 0xc837, LBC::h3}, {0xc838, 0xc838, LBC::h2}, {0xc839, 0xc853, LBC::h3}, {0xc854, 0xc854, LBC::h2},
	{0xc855, 0xc86f, LBC::h3}, {0xc870, 0xc870, LBC::h2}, {0xc871, 0xc88b, LBC::h3}, {0xc88c, 0xc88c, LBC::h2},
	{0xc88d, 0xc8a7, LBC::h3}, {0xc8a8, 0xc8a8, LBC::h2}, {0xc8a9, 0xc8c3, LBC::h3}, {0xc8c4, 0xc8c4, LBC::h2},
	{0xc8c5, 0xc8df, LBC::h3}, {0xc8e0, 0xc8e0, LBC::h2}, {0xc8e1, 0xc8fb, LBC::h3}, {0xc8fc, 0xc8fc, LBC::h2},
	{0xc8fd, 0xc917, LBC::h3}, {0xc918, 0xc918, LBC::h2}, {0xc919, 0xc933, LBC::h3}, {0xc934, 0xc934, LBC::h2},
	{0xc935, 0xc94f, LBC::h3}, {0xc950, 0xc950, LBC::h2}, {0xc951, 0xc96b, LBC::h3}, {0xc96c, 0xc96c, LBC::h2},
	{0xc96d, 0xc987, LBC::h3}, {0xc988, 0xc988, LBC::h2}, {0xc989, 0xc9a3, LBC::h3}, {0xc9a4, 0xc9a4, LBC::h2},
	{0xc9a5, 0xc9bf, LBC::h3}, {0xc9c0, 0xc9c0, LBC::h2}, {0xc9c1, 0xc9db, LBC::h3}, {0xc9dc, 0xc9dc, LBC::h2},
	{0xc9dd, 0xc9f7, LBC::h3}, {0xc9f8, 0xc9f8, LBC::h2}, {0xc9f9, 0xca13, LBC::h3}, {0xca14, 0xca14, LBC::h2},
	{0xca15, 0xca2f, LBC::h3}, {0xca30, 0xca30, LBC::h2}, {0xca31, 0xca4b, LBC::h3}, {0xca4c, 0xca4c, LBC::h2},
	{0xca4d, 0xca67, LBC::h3}, {0xca68, 0xca68, LBC::h2}, {0xca69, 0xca83, LBC::h3}, {0xca84, 0xca84, LBC::h2},
	{0xca85, 0xca9f, LBC::h3}, {0xcaa0, 0xcaa0, LBC::h2}, {0xcaa1, 0xcabb, LBC::h3}, {0xcabc, 0xcabc, LBC::h2},
	{0xcabd, 0xcad7, LBC::h3}, {0xcad8, 0xcad8, LBC::h2}, {0xcad9, 0xcaf3, LBC::h3}, {0xcaf4, 0xcaf4, LBC::h2},
	{0xcaf5, 0xcb0f, LBC::h3}, {0xcb10, 0xcb10, LBC::h2}, {0xcb11, 0xcb2b, LBC::h3}, {0xcb2c, 0xcb2c, LBC::h2},
	{0xcb2d, 0xcb47, LBC::h3}, {0xcb48, 0xcb48, LBC::h2}, {0xcb49, 0xcb63, LBC::h3}, {0xcb64, 0xcb64, LBC::h2},
	{0xcb65, 0xcb7f, LBC::h3}, {0xcb80, 0xcb80, LBC::h2}, {0xcb81, 0xcb9b, LBC::h3}, {0xcb9c, 0xcb9c, LBC::h2},
	{0xcb9d, 0xcbb7, LBC::h3}, {0xcbb8, 0xcbb8, LBC::h2}, {0xcbb9, 0xcbd3, LBC::h3}, {0xcbd4, 0xcbd4, LBC::h2},
	{0xcbd5, 0xcbef, LBC::h3}, {0xcbf0, 0xcbf0, LBC::h2}, {0xcbf1, 0xcc0b, LBC::h3}, {0xcc0c, 0xcc0c, LBC::h2},
	{0xcc0d, 0xcc27, LBC::h3}, {0xcc28, 0xcc28, LBC::h2}, {0xcc29, 0xcc43, LBC::h3}, {0xcc44, 0xcc44, LBC::h2},
	{0xcc45, 0xcc5f, LBC::h3}, {0xcc60, 0xcc60, LBC::h2}, {0xcc61, 0xcc7b, LBC::h3}, {0xcc7c, 0xcc7c, LBC::h2},
	{0xcc7d, 0xcc97, LBC::h3}, {0xcc98, 0xcc98, LBC::h2}, {0xcc99, 0xccb3, LBC::h3}, {0xccb4, 0xccb4, LBC::h2},
	{0xccb5, 0xcccf, LBC::h3}, {0xccd0, 0xccd0, LBC::h2}, {0xccd1, 0xcceb, LBC::h3}, {0xccec, 0xccec, LBC::h2},
	{0xcced, 0xcd07, LBC::h3}, {0xcd08, 0xcd08, LBC::h2}, {0xcd09, 0xcd23, LBC::h3}, {0xcd24, 0xcd24, LBC::h2},
	{0xcd25, 0xcd3f, LBC::h3}, {0xcd40, 0xcd40, LBC::h2}, {0xcd41, 0xcd5b, LBC::h3}, {0xcd5c, 0xcd5c, LBC::h2},
	{0xcd5d, 0xcd77, LBC::h3}, {0xcd78, 0xcd78, LBC::h2}, {0xcd79, 0xcd93, LBC::h3}, {0xcd94, 0xcd94, LBC::h2},
	{0xcd95, 0xcdaf, LBC::h3}, {0xcdb0, 0xcdb0, LBC::h2}, {0xcdb1, 0xcdcb, LBC::h3}, {0xcdcc, 0xcdcc, LBC::h2},
	{0xcdcd, 0xcde7, LBC::h3}, {0xcde8, 0xcde8, LBC::h2}, {0xcde9, 0xce03, LBC::h3}, {0xce04, 0xce04, LBC::h2},
	{0xce05, 0xce1f, LBC::h3}, {0xce20, 0xce20, LBC::h2}, {0xce21, 0xce3b, LBC::h3}, {0xce3c, 0xce3c, LBC::h2},
	{0xce3d, 0xce57, LBC::h3}, {0xce58, 0xce58, LBC::h2}, {0xce59, 0xce73, LBC::h3}, {0xce74, 0xce74, LBC::h2},
	{0xce75, 0xce8f, LBC::h3}, {0xce90, 0xce90, LBC::h2}, {0xce91, 0xceab, LBC::h3}, {0xceac, 0xceac, LBC::h2},
	{0xcead, 0xcec7, LBC::h3}, {0xcec8, 0xcec8, LBC::h2}, {0xcec9, 0xcee3, LBC::h3}, {0xcee4, 0xcee4, LBC::h2},
	{0xcee5, 0xceff, LBC::h3}, {0xcf00, 0xcf00, LBC::h2}, {0xcf01, 0xcf1b, LBC::h3}, {0xcf1c, 0xcf1c, LBC::h2},
	{0xcf1d, 0xcf37, LBC::h3}, {0xcf38, 0xcf38, LBC::h2}, {0xcf39, 0xcf53, LBC::h3}, {0xcf54, 0xcf54, LBC::h2},
	{0xcf55, 0xcf6f, LBC::h3}, {0xcf70, 0xcf70, LBC::h2}, {0xcf71, 0xcf8b, LBC::h3}, {0xcf8c, 0xcf8c, LBC::h2},
	{0xcf8d, 0xcfa7, LBC::h3}, {0xcfa8, 0xcfa8, LBC::h2}, {0xcfa9, 0xcfc3, LBC::h3}, {0xcfc4, 0xcfc4, LBC::h2},
	{0xcfc5, 0xcfdf, LBC::h3}, {0xcfe0, 0xcfe0, LBC::h2}, {0xcfe1, 0xcffb, LBC::h3}, {0xcffc, 0xcffc, LBC::h2},
	{0xcffd, 0xd017, LBC::h3}, {0xd018, 0xd018, LBC::h2}, {0xd019, 0xd033, LBC::h3}, {0xd034, 0xd034, LBC::h2},
	{0xd035, 0xd04f, LBC::h3}, {0xd050, 0xd050, LBC::h2}, {0xd051, 0xd06b, LBC::h3}, {0xd06c, 0xd06c, LBC::h2},
	{0xd06d, 0xd087, LBC::h3}, {0xd088, 0xd088, LBC::h2}, {0xd089, 0xd0a3, LBC::h3}, {0xd0a4, 0xd0a4, LBC::h2},
	{0xd0a5, 0xd0bf, LBC::h3}, {0xd0c0, 0xd0c0, LBC::h2}, {0xd0c1, 0xd0db, LBC::h3}, {0xd0dc, 0xd0dc, LBC::h2},
	{0xd0dd, 0xd0f7, LBC::h3}, {0xd0f8, 0xd0f8, LBC::h2}, {0xd0f9, 0xd113, LBC::h3}, {0xd114, 0xd114, LBC::h2},
	{0xd115, 0xd12f, LBC::h3}, {0xd130, 0xd130, LBC::h2}, {0xd131, 0xd14b, LBC::h3}, {0xd14c, 0xd14c, LBC::h2},
	{0xd14d, 0xd167, LBC::h3}, {0xd168, 0xd168, LBC::h2}, {0xd169, 0xd183, LBC::h3}, {0xd184, 0xd184, LBC::h2},
	{0xd185, 0xd19f, LBC::h3}, {0xd1a0, 0xd1a0, LBC::h2}, {0xd1a1, 0xd1bb, LBC::h3}, {0xd1bc, 0xd1bc, LBC::h2},
	{0xd1bd, 0xd1d7, LBC::h3}, {0xd1d8, 0xd1d8, LBC::h2}, {0xd1d9, 0xd1f3, LBC::h3}, {0xd1f4, 0xd1f4, LBC::h2},
	{0xd1f5, 0xd20f, LBC::h3}, {0xd210, 0xd210, LBC::h2}, {0xd211, 0xd22b, LBC::h3}, {0xd22c, 0xd22c, LBC::h2},
	{0xd22d, 0xd247, LBC::h3}, {0xd248, 0xd248, LBC::h2}, {0xd249, 0xd263, LBC::h3}, {0xd264, 0xd264, LBC::h2},
	{0xd265, 0xd27f, LBC::h3}, {0xd280, 0xd280, LBC::h2}, {0xd281, 0xd29b, LBC::h3}, {0xd29c, 0xd29c, LBC::h2},
	{0xd29d, 0xd2b7, LBC::h3}, {0xd2b8, 0xd2b8, LBC::h2}, {0xd2b9, 0xd2d3, LBC::h3}, {0xd2d4, 0xd2d4, LBC::h2},
	{0xd2d5, 0xd2ef, LBC::h3}, {0xd2f0, 0xd2f0, LBC::h2}, {0xd2f1, 0xd30b, LBC::h3}, {0xd30c, 0xd30c, LBC::h2},
	{0xd30d, 0xd327, LBC::h3}, {0xd328, 0xd328, LBC::h2}, {0xd329, 0xd343, LBC::h3}, {0xd344, 0xd344, LBC::h2},
	{0xd345, 0xd35f, LBC::h3}, {0xd360, 0xd360, LBC::h2}, {0xd361, 0xd37b, LBC::h3}, {0xd37c, 0xd37c, LBC::h2},
	{0xd37d, 0xd397, LBC::h3}, {0xd398, 0xd398, LBC::h2}, {0xd399, 0xd3b3, LBC::h3}, {0xd3b4, 0xd3b4, LBC::h2},
	{0xd3b5, 0xd3cf, LBC::h3}, {0xd3d0, 0xd3d0, LBC::h2}, {0xd3d1, 0xd3eb, LBC::h3}, {0xd3ec, 0xd3ec, LBC::h2},
	{0xd3ed, 0xd407, LBC::h3}, {0xd408, 0xd408, LBC::h2}, {0xd409, 0xd423, LBC::h3}, {0xd424, 0xd424, LBC::h2},
	{0xd425, 0xd43f, LBC::h3}, {0xd440, 0xd440, LBC::h2}, {0xd441, 0xd45b, LBC::h3}, {0xd45c, 0xd45c, LBC::h2},
	{0xd45d, 0xd477, LBC::h3}, {0xd478, 0xd478, LBC::h2}, {0xd479, 0xd493, LBC::h3}, {0xd494, 0xd494, LBC::h2},
	{0xd495, 0xd4af, LBC::h3}, {0xd4b0, 0xd4b0, LBC::h2}, {0xd4b1, 0xd4cb, LBC::h3}, {0xd4cc, 0xd4cc, LBC::h2},
	{0xd4cd, 0xd4e7, LBC::h3}, {0xd4e8, 0xd4e8, LBC::h2}, {0xd4e9, 0xd503, LBC::h3}, {0xd504, 0xd504, LBC::h2},
	{0xd505, 0xd51f, LBC::h3}, {0xd520, 0xd520, LBC::h2}, {0xd521, 0xd53b, LBC::h3}, {0xd53c, 0xd53c, LBC::h2},
	{0xd53d, 0xd557, LBC::h3}, {0xd558, 0xd558, LBC::h2}, {0xd559, 0xd573, LBC::h3}, {0xd574, 0xd574, LBC::h2},
	{0xd575, 0xd58f, LBC::h3}, {0xd590, 0xd590, LBC::h2}, {0xd591, 0xd5ab, LBC::h3}, {0xd5ac, 0xd5ac, LBC::h2},
	{0xd5ad, 0xd5c7, LBC::h3}, {0xd5c8, 0xd5c8, LBC::h2}, {0xd5c9, 0xd5e3, LBC::h3}, {0xd5e4, 0xd5e4, LBC::h2},
	{0xd5e5, 0xd5ff, LBC::h3}, {0xd600, 0xd600, LBC::h2}, {0xd601, 0xd61b, LBC::h3}, {0xd61c, 0xd61c, LBC::h2},
	{0xd61d, 0xd637, LBC::h3}, {0xd638, 0xd638, LBC::h2}, {0xd639, 0xd653, LBC::h3}, {0xd654, 0xd654, LBC::h2},
	{0xd655, 0xd66f, LBC::h3}, {0xd670, 0xd670, LBC::h2}, {0xd671, 0xd68b, LBC::h3}, {0xd68c, 0xd68c, LBC::h2},
	{0xd68d, 0xd6a7, LBC::h3}, {0xd6a8, 0xd6a8, LBC::h2}, {0xd6a9, 0xd6c3, LBC::h3}, {0xd6c4, 0xd6c4, LBC::h2},
	{0xd6c5, 0xd6df, LBC::h3}, {0xd6e0, 0xd6e0, LBC::h2}, {0xd6e1, 0xd6fb, LBC::h3}, {0xd6fc, 0xd6fc, LBC::h2},
	{0xd6fd, 0xd717, LBC::h3}, {0xd718, 0xd718, LBC::h2}, {0xd719, 0xd733, LBC::h3}, {0xd734, 0xd734, LBC::h2},
	{0xd735, 0xd74f, LBC::h3}, {0xd750, 0xd750, LBC::h2}, {0xd751, 0xd76b, LBC::h3}, {0xd76c, 0xd76c, LBC::h2},
	{0xd76d, 0xd787, LBC::h3}, {0xd788, 0xd788, LBC::h2}, {0xd789, 0xd7a3, LBC::h3}, {0xd7b0, 0xd7c6, LBC::jv},
	{0xd7cb, 0xd7fb, LBC::jt}, {0xd800, 0xdfff, LBC::sg}, {0xe000, 0xf8ff, LBC::xx}, {0xf900, 0xfaff, LBC::id},
	{0xfb00, 0xfb06, LBC::al}, {0xfb13, 0xfb17, LBC::al}, {0xfb1d, 0xfb1d, LBC::hl}, {0xfb1e, 0xfb1e, LBC::cm},
	{0xfb1f, 0xfb28, LBC::hl}, {0xfb29, 0xfb29, LBC::al}, {0xfb2a, 0xfb36, LBC::hl}, {0xfb38, 0xfb3c, LBC::hl},
	{0xfb3e, 0xfb3e, LBC::hl}, {0xfb40, 0xfb41, LBC::hl}, {0xfb43, 0xfb44, LBC::hl}, {0xfb46, 0xfb4f, LBC::hl},
	{0xfb50, 0xfd3d, LBC::al}, {0xfd3e, 0xfd3e, LBC::cl}, {0xfd3f, 0xfd3f, LBC::op}, {0xfd40, 0xfdcf, LBC::al},
	{0xfdf0, 0xfdfb, LBC::al}, {0xfdfc, 0xfdfc, LBC::po}, {0xfdfd, 0xfdff, LBC::al}, {0xfe00, 0xfe0f, LBC::cm},
	{0xfe10, 0xfe12, LBC::cl}, {0xfe13, 0xfe14, LBC::ns}, {0xfe15, 0xfe16, LBC::ex}, {0xfe17, 0xfe17, LBC::op},
	{0xfe18, 0xfe18, LBC::cl}, {0xfe19, 0xfe19, LBC::in}, {0xfe20, 0xfe20, LBC::gl}, {0xfe21, 0xfe21, LBC::cm},
	{0xfe22, 0xfe22, LBC::gl}, {0xfe23, 0xfe23, LBC::cm}, {0xfe24, 0xfe24, LBC::gl}, {0xfe25, 0xfe25, LBC::cm},
	{0xfe26, 0xfe27, LBC::gl}, {0xfe28, 0xfe28, LBC::cm}, {0xfe29, 0xfe29, LBC::gl}, {0xfe2a, 0xfe2a, LBC::cm},
	{0xfe2b, 0xfe2b, LBC::gl}, {0xfe2c, 0xfe2c, LBC::cm}, {0xfe2d, 0xfe2e, LBC::gl}, {0xfe2f, 0xfe2f, LBC::cm},
	{0xfe30, 0xfe34, LBC::id}, {0xfe35, 0xfe35, LBC::op}, {0xfe36, 0xfe36, LBC::cl}, {0xfe37, 0xfe37, LBC::op},
	{0xfe38, 0xfe38, LBC::cl}, {0xfe39, 0xfe39, LBC::op}, {0xfe3a, 0xfe3a, LBC::cl}, {0xfe3b, 0xfe3b, LBC::op},
	{0xfe3c, 0xfe3c, LBC::cl}, {0xfe3d, 0xfe3d, LBC::op}, {0xfe3e, 0xfe3e, LBC::cl}, {0xfe3f, 0xfe3f, LBC::op},
	{0xfe40, 0xfe40, LBC::cl}, {0xfe41, 0xfe41, LBC::op}, {0xfe42, 0xfe42, LBC::cl}, {0xfe43, 0xfe43, LBC::op},
	{0xfe44, 0xfe44, LBC::cl}, {0xfe45, 0xfe46, LBC::id}, {0xfe47, 0xfe47, LBC::op}, {0xfe48, 0xfe48, LBC::cl},
	{0xfe49, 0xfe4f, LBC::id}, {0xfe50, 0xfe52, LBC::cl}, {0xfe54, 0xfe55, LBC::ns}, {0xfe56, 0xfe57, LBC::ex},
	{0xfe58, 0xfe58, LBC::id}, {0xfe59, 0xfe59, LBC::op}, {0xfe5a, 0xfe5a, LBC::cl}, {0xfe5b, 0xfe5b, LBC::op},
	{0xfe5c, 0xfe5c, LBC::cl}, {0xfe5d, 0xfe5d, LBC::op}, {0xfe5e, 0xfe5e, LBC::cl}, {0xfe5f, 0xfe66, LBC::id},
	{0xfe68, 0xfe68, LBC::id}, {0xfe69, 0xfe69, LBC::pr}, {0xfe6a, 0xfe6a, LBC::po}, {0xfe6b, 0xfe6b, LBC::id},
	{0xfe70, 0xfe74, LBC::al}, {0xfe76, 0xfefc, LBC::al}, {0xfeff, 0xfeff, LBC::wj}, {0xff01, 0xff01, LBC::ex},
	{0xff02, 0xff03, LBC::id}, {0xff04, 0xff04, LBC::pr}, {0xff05, 0xff05, LBC::po}, {0xff06, 0xff07, LBC::id},
	{0xff08, 0xff08, LBC::op}, {0xff09, 0xff09, LBC::cl}, {0xff0a, 0xff0b, LBC::id}, {0xff0c, 0xff0c, LBC::cl},
	{0xff0d, 0xff0d, LBC::id}, {0xff0e, 0xff0e, LBC::cl}, {0xff0f, 0xff19, LBC::id}, {0xff1a, 0xff1b, LBC::ns},
	{0xff1c, 0xff1e, LBC::id}, {0xff1f, 0xff1f, LBC::ex}, {0xff20, 0xff3a, LBC::id}, {0xff3b, 0xff3b, LBC::op},
	{0xff3c, 0xff3c, LBC::id}, {0xff3d, 0xff3d, LBC::cl}, {0xff3e, 0xff5a, LBC::id}, {0xff5b, 0xff5b, LBC::op},
	{0xff5c, 0xff5c, LBC::id}, {0xff5d, 0xff5d, LBC::cl}, {0xff5e, 0xff5e, LBC::id}, {0xff5f, 0xff5f, LBC::op},
	{0xff60, 0xff61, LBC::cl}, {0xff62, 0xff62, LBC::op}, {0xff63, 0xff64, LBC::cl}, {0xff65, 0xff65, LBC::ns},
	{0xff66, 0xff66, LBC::id}, {0xff67, 0xff70, LBC::cj}, {0xff71, 0xff9d, LBC::id}, {0xff9e, 0xff9f, LBC::ns},
	{0xffa0, 0xffbe, LBC::id}, {0xffc2, 0xffc7, LBC::id}, {0xffca, 0xffcf, LBC::id}, {0xffd2, 0xffd7, LBC::id},
	{0xffda, 0xffdc, LBC::id}, {0xffe0, 0xffe0, LBC::po}, {0xffe1, 0xffe1, LBC::pr}, {0xffe2, 0xffe4, LBC::id},
	{0xffe5, 0xffe6, LBC::pr}, {0xffe8, 0xffee, LBC::al}, {0xfff9, 0xfffb, LBC::cm}, {0xfffc, 0xfffc, LBC::cb},
	{0xfffd, 0xfffd, LBC::ai}
};


//
//	lineBreak32
//

#if defined(IMGUI_USE_WCHAR32)

static LineBreakRange32 lineBreak32[] = {
	{0x10000, 0x1000b, LBC::al}, {0x1000d, 0x10026, LBC::al}, {0x10028, 0x1003a, LBC::al}, {0x1003c, 0x1003d, LBC::al},
	{0x1003f, 0x1004d, LBC::al}, {0x10050, 0x1005d, LBC::al}, {0x10080, 0x100fa, LBC::al}, {0x10100, 0x10102, LBC::ba},
	{0x10107, 0x10133, LBC::al}, {0x10137, 0x1018e, LBC::al}, {0x10190, 0x1019c, LBC::al}, {0x101a0, 0x101a0, LBC::al},
	{0x101d0, 0x101fc, LBC::al}, {0x101fd, 0x101fd, LBC::cm}, {0x10280, 0x1029c, LBC::al}, {0x102a0, 0x102d0, LBC::al},
	{0x102e0, 0x102e0, LBC::cm}, {0x102e1, 0x102fb, LBC::al}, {0x10300, 0x10323, LBC::al}, {0x1032d, 0x1034a, LBC::al},
	{0x10350, 0x10375, LBC::al}, {0x10376, 0x1037a, LBC::cm}, {0x10380, 0x1039d, LBC::al}, {0x1039f, 0x1039f, LBC::ba},
	{0x103a0, 0x103c3, LBC::al}, {0x103c8, 0x103cf, LBC::al}, {0x103d0, 0x103d0, LBC::ba}, {0x103d1, 0x103d5, LBC::al},
	{0x10400, 0x1049d, LBC::al}, {0x104a0, 0x104a9, LBC::nu}, {0x104b0, 0x104d3, LBC::al}, {0x104d8, 0x104fb, LBC::al},
	{0x10500, 0x10527, LBC::al}, {0x10530, 0x10563, LBC::al}, {0x1056f, 0x1057a, LBC::al}, {0x1057c, 0x1058a, LBC::al},
	{0x1058c, 0x10592, LBC::al}, {0x10594, 0x10595, LBC::al}, {0x10597, 0x105a1, LBC::al}, {0x105a3, 0x105b1, LBC::al},
	{0x105b3, 0x105b9, LBC::al}, {0x105bb, 0x105bc, LBC::al}, {0x105c0, 0x105f3, LBC::al}, {0x10600, 0x10736, LBC::al},
	{0x10740, 0x10755, LBC::al}, {0x10760, 0x10767, LBC::al}, {0x10780, 0x10785, LBC::al}, {0x10787, 0x107b0, LBC::al},
	{0x107b2, 0x107bf, LBC::al}, {0x10800, 0x10805, LBC::al}, {0x10808, 0x10808, LBC::al}, {0x1080a, 0x10835, LBC::al},
	{0x10837, 0x10838, LBC::al}, {0x1083c, 0x1083c, LBC::al}, {0x1083f, 0x10855, LBC::al}, {0x10857, 0x10857, LBC::ba},
	{0x10858, 0x1089e, LBC::al}, {0x108a7, 0x108af, LBC::al}, {0x108e0, 0x108f2, LBC::al}, {0x108f4, 0x108f5, LBC::al},
	{0x108fb, 0x1091b, LBC::al}, {0x1091f, 0x1091f, LBC::ba}, {0x10920, 0x10939, LBC::al}, {0x1093f, 0x10959, LBC::al},
	{0x10980, 0x109b7, LBC::al}, {0x109bc, 0x109cf, LBC::al}, {0x109d2, 0x10a00, LBC::al}, {0x10a01, 0x10a03, LBC::cm},
	{0x10a05, 0x10a06, LBC::cm}, {0x10a0c, 0x10a0f, LBC::cm}, {0x10a10, 0x10a13, LBC::al}, {0x10a15, 0x10a17, LBC::al},
	{0x10a19, 0x10a35, LBC::al}, {0x10a38, 0x10a3a, LBC::cm}, {0x10a3f, 0x10a3f, LBC::cm}, {0x10a40, 0x10a48, LBC::al},
	{0x10a50, 0x10a57, LBC::ba}, {0x10a58, 0x10a58, LBC::al}, {0x10a60, 0x10a9f, LBC::al}, {0x10ac0, 0x10ae4, LBC::al},
	{0x10ae5, 0x10ae6, LBC::cm}, {0x10aeb, 0x10aef, LBC::al}, {0x10af0, 0x10af5, LBC::ba}, {0x10af6, 0x10af6, LBC::in},
	{0x10b00, 0x10b35, LBC::al}, {0x10b39, 0x10b3f, LBC::ba}, {0x10b40, 0x10b55, LBC::al}, {0x10b58, 0x10b72, LBC::al},
	{0x10b78, 0x10b91, LBC::al}, {0x10b99, 0x10b9c, LBC::al}, {0x10ba9, 0x10baf, LBC::al}, {0x10c00, 0x10c48, LBC::al},
	{0x10c80, 0x10cb2, LBC::al}, {0x10cc0, 0x10cf2, LBC::al}, {0x10cfa, 0x10d23, LBC::al}, {0x10d24, 0x10d27, LBC::cm},
	{0x10d30, 0x10d39, LBC::nu}, {0x10d40, 0x10d49, LBC::nu}, {0x10d4a, 0x10d65, LBC::al}, {0x10d69, 0x10d6d, LBC::cm},
	{0x10d6e, 0x10d6e, LBC::hh}, {0x10d6f, 0x10d85, LBC::al}, {0x10d8e, 0x10d8f, LBC::al}, {0x10e60, 0x10e7e, LBC::al},
	{0x10e80, 0x10ea9, LBC::al}, {0x10eab, 0x10eac, LBC::cm}, {0x10ead, 0x10ead, LBC::hh}, {0x10eb0, 0x10eb1, LBC::al},
	{0x10ec2, 0x10ec7, LBC::al}, {0x10ec9, 0x10eca, LBC::al}, {0x10ecb, 0x10ecf, LBC::cm}, {0x10ed0, 0x10ed0, LBC::ba},
	{0x10ed1, 0x10eee, LBC::al}, {0x10ef0, 0x10eff, LBC::cm}, {0x10f00, 0x10f27, LBC::al}, {0x10f30, 0x10f45, LBC::al},
	{0x10f46, 0x10f50, LBC::cm}, {0x10f51, 0x10f59, LBC::al}, {0x10f70, 0x10f81, LBC::al}, {0x10f82, 0x10f85, LBC::cm},
	{0x10f86, 0x10f89, LBC::al}, {0x10fb0, 0x10fcb, LBC::al}, {0x10fe0, 0x10ff6, LBC::al}, {0x11000, 0x11002, LBC::cm},
	{0x11003, 0x11004, LBC::ap}, {0x11005, 0x11037, LBC::ak}, {0x11038, 0x11045, LBC::cm}, {0x11046, 0x11046, LBC::vi},
	{0x11047, 0x11048, LBC::ba}, {0x11049, 0x1104d, LBC::id}, {0x11052, 0x11065, LBC::id}, {0x11066, 0x1106f, LBC::as},
	{0x11070, 0x11070, LBC::cm}, {0x11071, 0x11072, LBC::ak}, {0x11073, 0x11074, LBC::cm}, {0x11075, 0x11075, LBC::ak},
	{0x1107f, 0x1107f, LBC::gl}, {0x11080, 0x11082, LBC::cm}, {0x11083, 0x110af, LBC::al}, {0x110b0, 0x110ba, LBC::cm},
	{0x110bb, 0x110bc, LBC::al}, {0x110bd, 0x110bd, LBC::nu}, {0x110be, 0x110c1, LBC::ba}, {0x110c2, 0x110c2, LBC::cm},
	{0x110cd, 0x110cd, LBC::nu}, {0x110d0, 0x110e8, LBC::al}, {0x110f0, 0x110f9, LBC::nu}, {0x11100, 0x11102, LBC::cm},
	{0x11103, 0x11126, LBC::al}, {0x11127, 0x11134, LBC::cm}, {0x11136, 0x1113f, LBC::nu}, {0x11140, 0x11143, LBC::ba},
	{0x11144, 0x11144, LBC::al}, {0x11145, 0x11146, LBC::cm}, {0x11147, 0x11147, LBC::al}, {0x11150, 0x11172, LBC::al},
	{0x11173, 0x11173, LBC::cm}, {0x11174, 0x11174, LBC::al}, {0x11175, 0x11175, LBC::bb}, {0x11176, 0x11176, LBC::al},
	{0x11180, 0x11182, LBC::cm}, {0x11183, 0x111b2, LBC::al}, {0x111b3, 0x111c0, LBC::cm}, {0x111c1, 0x111c4, LBC::al},
	{0x111c5, 0x111c6, LBC::ba}, {0x111c7, 0x111c7, LBC::al}, {0x111c8, 0x111c8, LBC::ba}, {0x111c9, 0x111cc, LBC::cm},
	{0x111cd, 0x111cd, LBC::al}, {0x111ce, 0x111cf, LBC::cm}, {0x111d0, 0x111d9, LBC::nu}, {0x111da, 0x111da, LBC::al},
	{0x111db, 0x111db, LBC::bb}, {0x111dc, 0x111dc, LBC::al}, {0x111dd, 0x111df, LBC::ba}, {0x111e1, 0x111f4, LBC::al},
	{0x11200, 0x11211, LBC::al}, {0x11213, 0x1122b, LBC::al}, {0x1122c, 0x11237, LBC::cm}, {0x11238, 0x11239, LBC::ba},
	{0x1123a, 0x1123a, LBC::al}, {0x1123b, 0x1123c, LBC::ba}, {0x1123d, 0x1123d, LBC::al}, {0x1123e, 0x1123e, LBC::cm},
	{0x1123f, 0x11240, LBC::al}, {0x11241, 0x11241, LBC::cm}, {0x11280, 0x11286, LBC::al}, {0x11288, 0x11288, LBC::al},
	{0x1128a, 0x1128d, LBC::al}, {0x1128f, 0x1129d, LBC::al}, {0x1129f, 0x112a8, LBC::al}, {0x112a9, 0x112a9, LBC::ba},
	{0x112b0, 0x112de, LBC::al}, {0x112df, 0x112ea, LBC::cm}, {0x112f0, 0x112f9, LBC::nu}, {0x11300, 0x11303, LBC::cm},
	{0x11305, 0x1130c, LBC::ak}, {0x1130f, 0x11310, LBC::ak}, {0x11313, 0x11328, LBC::ak}, {0x1132a, 0x11330, LBC::ak},
	{0x11332, 0x11333, LBC::ak}, {0x11335, 0x11339, LBC::ak}, {0x1133b, 0x1133c, LBC::cm}, {0x1133d, 0x1133d, LBC::ba},
	{0x1133e, 0x11344, LBC::cm}, {0x11347, 0x11348, LBC::cm}, {0x1134b, 0x1134c, LBC::cm}, {0x1134d, 0x1134d, LBC::vi},
	{0x11350, 0x11350, LBC::as}, {0x11357, 0x11357, LBC::cm}, {0x1135d, 0x1135d, LBC::ba}, {0x1135e, 0x1135f, LBC::as},
	{0x11360, 0x11361, LBC::ak}, {0x11362, 0x11363, LBC::cm}, {0x11366, 0x1136c, LBC::cm}, {0x11370, 0x11374, LBC::cm},
	{0x11380, 0x11389, LBC::as}, {0x1138b, 0x1138b, LBC::as}, {0x1138e, 0x1138e, LBC::as}, {0x11390, 0x11391, LBC::as},
	{0x11392, 0x113b5, LBC::ak}, {0x113b7, 0x113b7, LBC::id}, {0x113b8, 0x113c0, LBC::cm}, {0x113c2, 0x113c2, LBC::cm},
	{0x113c5, 0x113c5, LBC::cm}, {0x113c7, 0x113ca, LBC::cm}, {0x113cc, 0x113cf, LBC::cm}, {0x113d0, 0x113d0, LBC::vi},
	{0x113d1, 0x113d1, LBC::ap}, {0x113d2, 0x113d2, LBC::cm}, {0x113d3, 0x113d5, LBC::id}, {0x113d7, 0x113d8, LBC::id},
	{0x113e1, 0x113e2, LBC::cm}, {0x11400, 0x11434, LBC::al}, {0x11435, 0x11446, LBC::cm}, {0x11447, 0x1144a, LBC::al},
	{0x1144b, 0x1144e, LBC::ba}, {0x1144f, 0x1144f, LBC::al}, {0x11450, 0x11459, LBC::nu}, {0x1145a, 0x1145b, LBC::ba},
	{0x1145d, 0x1145d, LBC::al}, {0x1145e, 0x1145e, LBC::cm}, {0x1145f, 0x11461, LBC::al}, {0x11480, 0x114af, LBC::al},
	{0x114b0, 0x114c3, LBC::cm}, {0x114c4, 0x114c7, LBC::al}, {0x114d0, 0x114d9, LBC::nu}, {0x11580, 0x115ae, LBC::al},
	{0x115af, 0x115b5, LBC::cm}, {0x115b8, 0x115c0, LBC::cm}, {0x115c1, 0x115c1, LBC::bb}, {0x115c2, 0x115c3, LBC::ba},
	{0x115c4, 0x115c5, LBC::ex}, {0x115c6, 0x115c8, LBC::al}, {0x115c9, 0x115d7, LBC::ba}, {0x115d8, 0x115db, LBC::al},
	{0x115dc, 0x115dd, LBC::cm}, {0x11600, 0x1162f, LBC::al}, {0x11630, 0x11640, LBC::cm}, {0x11641, 0x11642, LBC::ba},
	{0x11643, 0x11644, LBC::al}, {0x11650, 0x11659, LBC::nu}, {0x11660, 0x1166c, LBC::bb}, {0x11680, 0x116aa, LBC::al},
	{0x116ab, 0x116b7, LBC::cm}, {0x116b8, 0x116b9, LBC::al}, {0x116c0, 0x116c9, LBC::nu}, {0x116d0, 0x116e3, LBC::nu},
	{0x11700, 0x1171a, LBC::sa}, {0x1171d, 0x1172b, LBC::sa}, {0x11730, 0x11739, LBC::nu}, {0x1173a, 0x1173b, LBC::sa},
	{0x1173c, 0x1173e, LBC::ba}, {0x1173f, 0x11746, LBC::sa}, {0x11800, 0x1182b, LBC::al}, {0x1182c, 0x1183a, LBC::cm},
	{0x1183b, 0x1183b, LBC::al}, {0x118a0, 0x118df, LBC::al}, {0x118e0, 0x118e9, LBC::nu}, {0x118ea, 0x118f2, LBC::al},
	{0x118ff, 0x118ff, LBC::al}, {0x11900, 0x11906, LBC::ak}, {0x11909, 0x11909, LBC::ak}, {0x1190c, 0x11913, LBC::ak},
	{0x11915, 0x11916, LBC::ak}, {0x11918, 0x1192f, LBC::ak}, {0x11930, 0x11935, LBC::cm}, {0x11937, 0x11938, LBC::cm},
	{0x1193b, 0x1193d, LBC::cm}, {0x1193e, 0x1193e, LBC::vi}, {0x1193f, 0x1193f, LBC::ap}, {0x11940, 0x11940, LBC::cm},
	{0x11941, 0x11941, LBC::ap}, {0x11942, 0x11943, LBC::cm}, {0x11944, 0x11946, LBC::ba}, {0x11950, 0x11959, LBC::as},
	{0x119a0, 0x119a7, LBC::al}, {0x119aa, 0x119d0, LBC::al}, {0x119d1, 0x119d7, LBC::cm}, {0x119da, 0x119e0, LBC::cm},
	{0x119e1, 0x119e1, LBC::al}, {0x119e2, 0x119e2, LBC::bb}, {0x119e3, 0x119e3, LBC::al}, {0x119e4, 0x119e4, LBC::cm},
	{0x11a00, 0x11a00, LBC::al}, {0x11a01, 0x11a0a, LBC::cm}, {0x11a0b, 0x11a32, LBC::al}, {0x11a33, 0x11a39, LBC::cm},
	{0x11a3a, 0x11a3a, LBC::al}, {0x11a3b, 0x11a3e, LBC::cm}, {0x11a3f, 0x11a3f, LBC::bb}, {0x11a40, 0x11a40, LBC::al},
	{0x11a41, 0x11a44, LBC::ba}, {0x11a45, 0x11a45, LBC::bb}, {0x11a46, 0x11a46, LBC::al}, {0x11a47, 0x11a47, LBC::cm},
	{0x11a50, 0x11a50, LBC::al}, {0x11a51, 0x11a5b, LBC::cm}, {0x11a5c, 0x11a89, LBC::al}, {0x11a8a, 0x11a99, LBC::cm},
	{0x11a9a, 0x11a9c, LBC::ba}, {0x11a9d, 0x11a9d, LBC::al}, {0x11a9e, 0x11aa0, LBC::bb}, {0x11aa1, 0x11aa2, LBC::ba},
	{0x11ab0, 0x11af8, LBC::al}, {0x11b00, 0x11b09, LBC::bb}, {0x11b0a, 0x11b0a, LBC::al}, {0x11b60, 0x11b67, LBC::cm},
	{0x11bc0, 0x11be1, LBC::al}, {0x11bf0, 0x11bf9, LBC::nu}, {0x11c00, 0x11c08, LBC::al}, {0x11c0a, 0x11c2e, LBC::al},
	{0x11c2f, 0x11c36, LBC::cm}, {0x11c38, 0x11c3f, LBC::cm}, {0x11c40, 0x11c40, LBC::al}, {0x11c41, 0x11c45, LBC::ba},
	{0x11c50, 0x11c59, LBC::nu}, {0x11c5a, 0x11c6c, LBC::al}, {0x11c70, 0x11c70, LBC::bb}, {0x11c71, 0x11c71, LBC::ex},
	{0x11c72, 0x11c8f, LBC::al}, {0x11c92, 0x11ca7, LBC::cm}, {0x11ca9, 0x11cb6, LBC::cm}, {0x11d00, 0x11d06, LBC::al},
	{0x11d08, 0x11d09, LBC::al}, {0x11d0b, 0x11d30, LBC::al}, {0x11d31, 0x11d36, LBC::cm}, {0x11d3a, 0x11d3a, LBC::cm},
	{0x11d3c, 0x11d3d, LBC::cm}, {0x11d3f, 0x11d45, LBC::cm}, {0x11d46, 0x11d46, LBC::al}, {0x11d47, 0x11d47, LBC::cm},
	{0x11d50, 0x11d59, LBC::nu}, {0x11d60, 0x11d65, LBC::al}, {0x11d67, 0x11d68, LBC::al}, {0x11d6a, 0x11d89, LBC::al},
	{0x11d8a, 0x11d8e, LBC::cm}, {0x11d90, 0x11d91, LBC::cm}, {0x11d93, 0x11d97, LBC::cm}, {0x11d98, 0x11d98, LBC::al},
	{0x11da0, 0x11da9, LBC::nu}, {0x11db0, 0x11ddb, LBC::al}, {0x11de0, 0x11de9, LBC::nu}, {0x11df0, 0x11df0, LBC::cm},
	{0x11df1, 0x11df1, LBC::al}, {0x11ee0, 0x11ef1, LBC::as}, {0x11ef2, 0x11ef2, LBC::ba}, {0x11ef3, 0x11ef6, LBC::cm},
	{0x11ef7, 0x11ef8, LBC::ba}, {0x11f00, 0x11f01, LBC::cm}, {0x11f02, 0x11f02, LBC::ap}, {0x11f03, 0x11f03, LBC::cm},
	{0x11f04, 0x11f10, LBC::ak}, {0x11f12, 0x11f33, LBC::ak}, {0x11f34, 0x11f3a, LBC::cm}, {0x11f3e, 0x11f41, LBC::cm},
	{0x11f42, 0x11f42, LBC::vi}, {0x11f43, 0x11f44, LBC::ba}, {0x11f45, 0x11f4f, LBC::id}, {0x11f50, 0x11f59, LBC::as},
	{0x11f5a, 0x11f5a, LBC::cm}, {0x11fb0, 0x11fb0, LBC::al}, {0x11fc0, 0x11fdc, LBC::al}, {0x11fdd, 0x11fe0, LBC::po},
	{0x11fe1, 0x11ff1, LBC::al}, {0x11fff, 0x11fff, LBC::ba}, {0x12000, 0x12399, LBC::al}, {0x12400, 0x1246f, LBC::al},
	{0x12470, 0x12474, LBC::ba}, {0x12475, 0x12543, LBC::al}, {0x12550, 0x12686, LBC::al}, {0x12f90, 0x12ff2, LBC::al},
	{0x13000, 0x13257, LBC::al}, {0x13258, 0x1325a, LBC::op}, {0x1325b, 0x1325d, LBC::cl}, {0x1325e, 0x13281, LBC::al},
	{0x13282, 0x13282, LBC::cl}, {0x13283, 0x13285, LBC::al}, {0x13286, 0x13286, LBC::op}, {0x13287, 0x13287, LBC::cl},
	{0x13288, 0x13288, LBC::op}, {0x13289, 0x13289, LBC::cl}, {0x1328a, 0x13378, LBC::al}, {0x13379, 0x13379, LBC::op},
	{0x1337a, 0x1337b, LBC::cl}, {0x1337c, 0x1342e, LBC::al}, {0x1342f, 0x1342f, LBC::op}, {0x13430, 0x13436, LBC::gl},
	{0x13437, 0x13437, LBC::op}, {0x13438, 0x13438, LBC::cl}, {0x13439, 0x1343b, LBC::gl}, {0x1343c, 0x1343c, LBC::op},
	{0x1343d, 0x1343d, LBC::cl}, {0x1343e, 0x1343e, LBC::op}, {0x1343f, 0x1343f, LBC::cl}, {0x13440, 0x13440, LBC::cm},
	{0x13441, 0x13446, LBC::al}, {0x13447, 0x13455, LBC::cm}, {0x13460, 0x143fa, LBC::al}, {0x14400, 0x145cd, LBC::al},
	{0x145ce, 0x145ce, LBC::op}, {0x145cf, 0x145cf, LBC::cl}, {0x145d0, 0x14646, LBC::al}, {0x16100, 0x1611d, LBC::as},
	{0x1611e, 0x1612f, LBC::cm}, {0x16130, 0x16139, LBC::as}, {0x16800, 0x16a38, LBC::al}, {0x16a40, 0x16a5e, LBC::al},
	{0x16a60, 0x16a69, LBC::nu}, {0x16a6e, 0x16a6f, LBC::ba}, {0x16a70, 0x16abe, LBC::al}, {0x16ac0, 0x16ac9, LBC::nu},
	{0x16ad0, 0x16aed, LBC::al}, {0x16af0, 0x16af4, LBC::cm}, {0x16af5, 0x16af5, LBC::ba}, {0x16b00, 0x16b2f, LBC::al},
	{0x16b30, 0x16b36, LBC::cm}, {0x16b37, 0x16b39, LBC::ba}, {0x16b3a, 0x16b43, LBC::al}, {0x16b44, 0x16b44, LBC::ba},
	{0x16b45, 0x16b45, LBC::al}, {0x16b50, 0x16b59, LBC::nu}, {0x16b5b, 0x16b61, LBC::al}, {0x16b63, 0x16b77, LBC::al},
	{0x16b7d, 0x16b8f, LBC::al}, {0x16d40, 0x16d6d, LBC::al}, {0x16d6e, 0x16d6f, LBC::ba}, {0x16d70, 0x16d79, LBC::nu},
	{0x16e40, 0x16e96, LBC::al}, {0x16e97, 0x16e98, LBC::ba}, {0x16e99, 0x16e9a, LBC::al}, {0x16ea0, 0x16eb8, LBC::al},
	{0x16ebb, 0x16ed3, LBC::al}, {0x16f00, 0x16f4a, LBC::al}, {0x16f4f, 0x16f4f, LBC::cm}, {0x16f50, 0x16f50, LBC::al},
	{0x16f51, 0x16f87, LBC::cm}, {0x16f8f, 0x16f92, LBC::cm}, {0x16f93, 0x16f9f, LBC::al}, {0x16fe0, 0x16fe3, LBC::ns},
	{0x16fe4, 0x16fe4, LBC::gl}, {0x16ff0, 0x16ff1, LBC::cm}, {0x16ff2, 0x16ff3, LBC::ns}, {0x16ff4, 0x16ff6, LBC::id},
	{0x17000, 0x18aff, LBC::id}, {0x18b00, 0x18cda, LBC::al}, {0x18cff, 0x18cff, LBC::al}, {0x18d00, 0x18d20, LBC::id},
	{0x18d80, 0x18df2, LBC::id}, {0x18e00, 0x19191, LBC::id}, {0x191a0, 0x191d2, LBC::id}, {0x1aff0, 0x1aff3, LBC::al},
	{0x1aff5, 0x1affb, LBC::al}, {0x1affd, 0x1affe, LBC::al}, {0x1b000, 0x1b128, LBC::id}, {0x1b132, 0x1b132, LBC::cj},
	{0x1b150, 0x1b152, LBC::cj}, {0x1b155, 0x1b155, LBC::cj}, {0x1b164, 0x1b168, LBC::cj}, {0x1b170, 0x1b2fb, LBC::id},
	{0x1bc00, 0x1bc6a, LBC::al}, {0x1bc70, 0x1bc7c, LBC::al}, {0x1bc80, 0x1bc88, LBC::al}, {0x1bc90, 0x1bc99, LBC::al},
	{0x1bc9c, 0x1bc9c, LBC::al}, {0x1bc9d, 0x1bc9e, LBC::cm}, {0x1bc9f, 0x1bc9f, LBC::ba}, {0x1bca0, 0x1bca3, LBC::cm},
	{0x1cc00, 0x1ccef, LBC::al}, {0x1ccf0, 0x1ccf9, LBC::nu}, {0x1ccfa, 0x1ccfc, LBC::al}, {0x1cd00, 0x1ceb3, LBC::al},
	{0x1ceba, 0x1ced0, LBC::al}, {0x1ced2, 0x1ced4, LBC::al}, {0x1cedd, 0x1cefd, LBC::al}, {0x1cf00, 0x1cf2d, LBC::cm},
	{0x1cf30, 0x1cf46, LBC::cm}, {0x1cf50, 0x1cfc3, LBC::al}, {0x1d000, 0x1d0f5, LBC::al}, {0x1d100, 0x1d126, LBC::al},
	{0x1d127, 0x1d128, LBC::cm}, {0x1d129, 0x1d164, LBC::al}, {0x1d165, 0x1d169, LBC::cm}, {0x1d16a, 0x1d16c, LBC::al},
	{0x1d16d, 0x1d182, LBC::cm}, {0x1d183, 0x1d184, LBC::al}, {0x1d185, 0x1d18b, LBC::cm}, {0x1d18c, 0x1d1a9, LBC::al},
	{0x1d1aa, 0x1d1ad, LBC::cm}, {0x1d1ae, 0x1d241, LBC::al}, {0x1d242, 0x1d244, LBC::cm}, {0x1d245, 0x1d245, LBC::al},
	{0x1d250, 0x1d252, LBC::cm}, {0x1d253, 0x1d25a, LBC::al}, {0x1d25b, 0x1d25c, LBC::cm}, {0x1d25d, 0x1d25e, LBC::al},
	{0x1d25f, 0x1d25f, LBC::cm}, {0x1d260, 0x1d27f, LBC::al}, {0x1d280, 0x1d281, LBC::cm}, {0x1d2c0, 0x1d2d3, LBC::al},
	{0x1d2e0, 0x1d2f3, LBC::al}, {0x1d300, 0x1d356, LBC::al}, {0x1d360, 0x1d378, LBC::al}, {0x1d400, 0x1d454, LBC::al},
	{0x1d456, 0x1d49c, LBC::al}, {0x1d49e, 0x1d49f, LBC::al}, {0x1d4a2, 0x1d4a2, LBC::al}, {0x1d4a5, 0x1d4a6, LBC::al},
	{0x1d4a9, 0x1d4ac, LBC::al}, {0x1d4ae, 0x1d4b9, LBC::al}, {0x1d4bb, 0x1d4bb, LBC::al}, {0x1d4bd, 0x1d4c3, LBC::al},
	{0x1d4c5, 0x1d505, LBC::al}, {0x1d507, 0x1d50a, LBC::al}, {0x1d50d, 0x1d514, LBC::al}, {0x1d516, 0x1d51c, LBC::al},
	{0x1d51e, 0x1d539, LBC::al}, {0x1d53b, 0x1d53e, LBC::al}, {0x1d540, 0x1d544, LBC::al}, {0x1d546, 0x1d546, LBC::al},
	{0x1d54a, 0x1d550, LBC::al}, {0x1d552, 0x1d6a6, LBC::al}, {0x1d6a8, 0x1d7cb, LBC::al}, {0x1d7ce, 0x1d7ff, LBC::nu},
	{0x1d800, 0x1d9ff, LBC::al}, {0x1da00, 0x1da36, LBC::cm}, {0x1da37, 0x1da3a, LBC::al}, {0x1da3b, 0x1da6c, LBC::cm},
	{0x1da6d, 0x1da74, LBC::al}, {0x1da75, 0x1da75, LBC::cm}, {0x1da76, 0x1da83, LBC::al}, {0x1da84, 0x1da84, LBC::cm},
	{0x1da85, 0x1da86, LBC::al}, {0x1da87, 0x1da8a, LBC::ba}, {0x1da8b, 0x1da8b, LBC::al}, {0x1da9b, 0x1da9f, LBC::cm},
	{0x1daa1, 0x1daaf, LBC::cm}, {0x1db00, 0x1db1c, LBC::al}, {0x1df00, 0x1df81, LBC::al}, {0x1df90, 0x1df96, LBC::al},
	{0x1dfcd, 0x1dfff, LBC::al}, {0x1e000, 0x1e006, LBC::cm}, {0x1e008, 0x1e018, LBC::cm}, {0x1e01b, 0x1e021, LBC::cm},
	{0x1e023, 0x1e024, LBC::cm}, {0x1e026, 0x1e02a, LBC::cm}, {0x1e030, 0x1e06d, LBC::al}, {0x1e08f, 0x1e08f, LBC::cm},
	{0x1e100, 0x1e12c, LBC::al}, {0x1e130, 0x1e136, LBC::cm}, {0x1e137, 0x1e13d, LBC::al}, {0x1e140, 0x1e149, LBC::nu},
	{0x1e14e, 0x1e14f, LBC::al}, {0x1e290, 0x1e2ad, LBC::al}, {0x1e2ae, 0x1e2ae, LBC::cm}, {0x1e2c0, 0x1e2eb, LBC::al},
	{0x1e2ec, 0x1e2ef, LBC::cm}, {0x1e2f0, 0x1e2f9, LBC::nu}, {0x1e2ff, 0x1e2ff, LBC::pr}, {0x1e4d0, 0x1e4eb, LBC::al},
	{0x1e4ec, 0x1e4ef, LBC::cm}, {0x1e4f0, 0x1e4f9, LBC::nu}, {0x1e5d0, 0x1e5ed, LBC::al}, {0x1e5ee, 0x1e5ef, LBC::cm},
	{0x1e5f0, 0x1e5f0, LBC::al}, {0x1e5f1, 0x1e5fa, LBC::nu}, {0x1e5ff, 0x1e5ff, LBC::al}, {0x1e6c0, 0x1e6de, LBC::al},
	{0x1e6e0, 0x1e6e2, LBC::al}, {0x1e6e3, 0x1e6e3, LBC::cm}, {0x1e6e4, 0x1e6e5, LBC::al}, {0x1e6e6, 0x1e6e6, LBC::cm},
	{0x1e6e7, 0x1e6ed, LBC::al}, {0x1e6ee, 0x1e6ef, LBC::cm}, {0x1e6f0, 0x1e6f4, LBC::al}, {0x1e6f5, 0x1e6f5, LBC::cm},
	{0x1e6fe, 0x1e6ff, LBC::al}, {0x1e7e0, 0x1e7e6, LBC::al}, {0x1e7e8, 0x1e7eb, LBC::al}, {0x1e7ed, 0x1e7ee, LBC::al},
	{0x1e7f0, 0x1e7fe, LBC::al}, {0x1e800, 0x1e8c4, LBC::al}, {0x1e8c7, 0x1e8cf, LBC::al}, {0x1e8d0, 0x1e8d6, LBC::cm},
	{0x1e900, 0x1e943, LBC::al}, {0x1e944, 0x1e94a, LBC::cm}, {0x1e94b, 0x1e94b, LBC::al}, {0x1e950, 0x1e959, LBC::nu},
	{0x1e95e, 0x1e95f, LBC::op}, {0x1ec71, 0x1ecab, LBC::al}, {0x1ecac, 0x1ecac, LBC::po}, {0x1ecad, 0x1ecaf, LBC::al},
	{0x1ecb0, 0x1ecb0, LBC::po}, {0x1ecb1, 0x1ecb4, LBC::al}, {0x1ed01, 0x1ed3d, LBC::al}, {0x1ee00, 0x1ee03, LBC::al},
	{0x1ee05, 0x1ee1f, LBC::al}, {0x1ee21, 0x1ee22, LBC::al}, {0x1ee24, 0x1ee24, LBC::al}, {0x1ee27, 0x1ee27, LBC::al},
	{0x1ee29, 0x1ee32, LBC::al}, {0x1ee34, 0x1ee37, LBC::al}, {0x1ee39, 0x1ee39, LBC::al}, {0x1ee3b, 0x1ee3b, LBC::al},
	{0x1ee42, 0x1ee42, LBC::al}, {0x1ee47, 0x1ee47, LBC::al}, {0x1ee49, 0x1ee49, LBC::al}, {0x1ee4b, 0x1ee4b, LBC::al},
	{0x1ee4d, 0x1ee4f, LBC::al}, {0x1ee51, 0x1ee52, LBC::al}, {0x1ee54, 0x1ee54, LBC::al}, {0x1ee57, 0x1ee57, LBC::al},
	{0x1ee59, 0x1ee59, LBC::al}, {0x1ee5b, 0x1ee5b, LBC::al}, {0x1ee5d, 0x1ee5d, LBC::al}, {0x1ee5f, 0x1ee5f, LBC::al},
	{0x1ee61, 0x1ee62, LBC::al}, {0x1ee64, 0x1ee64, LBC::al}, {0x1ee67, 0x1ee6a, LBC::al}, {0x1ee6c, 0x1ee72, LBC::al},
	{0x1ee74, 0x1ee77, LBC::al}, {0x1ee79, 0x1ee7c, LBC::al}, {0x1ee7e, 0x1ee7e, LBC::al}, {0x1ee80, 0x1ee89, LBC::al},
	{0x1ee8b, 0x1ee9b, LBC::al}, {0x1eea1, 0x1eea3, LBC::al}, {0x1eea5, 0x1eea9, LBC::al}, {0x1eeab, 0x1eebb, LBC::al},
	{0x1eef0, 0x1eef1, LBC::al}, {0x1f000, 0x1f0ff, LBC::id}, {0x1f100, 0x1f10c, LBC::ai}, {0x1f10d, 0x1f10f, LBC::al},
	{0x1f110, 0x1f12d, LBC::ai}, {0x1f12e, 0x1f12f, LBC::al}, {0x1f130, 0x1f169, LBC::ai}, {0x1f16a, 0x1f16f, LBC::al},
	{0x1f170, 0x1f1ac, LBC::ai}, {0x1f1ad, 0x1f1ad, LBC::al}, {0x1f1ae, 0x1f1e5, LBC::id}, {0x1f1e6, 0x1f1ff, LBC::ri},
	{0x1f200, 0x1f384, LBC::id}, {0x1f385, 0x1f385, LBC::eb}, {0x1f386, 0x1f39b, LBC::id}, {0x1f39c, 0x1f39d, LBC::al},
	{0x1f39e, 0x1f3b4, LBC::id}, {0x1f3b5, 0x1f3b6, LBC::al}, {0x1f3b7, 0x1f3bb, LBC::id}, {0x1f3bc, 0x1f3bc, LBC::al},
	{0x1f3bd, 0x1f3c1, LBC::id}, {0x1f3c2, 0x1f3c4, LBC::eb}, {0x1f3c5, 0x1f3c6, LBC::id}, {0x1f3c7, 0x1f3c7, LBC::eb},
	{0x1f3c8, 0x1f3c9, LBC::id}, {0x1f3ca, 0x1f3cc, LBC::eb}, {0x1f3cd, 0x1f3fa, LBC::id}, {0x1f3fb, 0x1f3ff, LBC::em},
	{0x1f400, 0x1f441, LBC::id}, {0x1f442, 0x1f443, LBC::eb}, {0x1f444, 0x1f445, LBC::id}, {0x1f446, 0x1f450, LBC::eb},
	{0x1f451, 0x1f465, LBC::id}, {0x1f466, 0x1f478, LBC::eb}, {0x1f479, 0x1f47b, LBC::id}, {0x1f47c, 0x1f47c, LBC::eb},
	{0x1f47d, 0x1f480, LBC::id}, {0x1f481, 0x1f483, LBC::eb}, {0x1f484, 0x1f484, LBC::id}, {0x1f485, 0x1f487, LBC::eb},
	{0x1f488, 0x1f48e, LBC::id}, {0x1f48f, 0x1f48f, LBC::eb}, {0x1f490, 0x1f490, LBC::id}, {0x1f491, 0x1f491, LBC::eb},
	{0x1f492, 0x1f49f, LBC::id}, {0x1f4a0, 0x1f4a0, LBC::al}, {0x1f4a1, 0x1f4a1, LBC::id}, {0x1f4a2, 0x1f4a2, LBC::al},
	{0x1f4a3, 0x1f4a3, LBC::id}, {0x1f4a4, 0x1f4a4, LBC::al}, {0x1f4a5, 0x1f4a9, LBC::id}, {0x1f4aa, 0x1f4aa, LBC::eb},
	{0x1f4ab, 0x1f4ae, LBC::id}, {0x1f4af, 0x1f4af, LBC::al}, {0x1f4b0, 0x1f4b0, LBC::id}, {0x1f4b1, 0x1f4b2, LBC::al},
	{0x1f4b3, 0x1f4ff, LBC::id}, {0x1f500, 0x1f506, LBC::al}, {0x1f507, 0x1f516, LBC::id}, {0x1f517, 0x1f524, LBC::al},
	{0x1f525, 0x1f531, LBC::id}, {0x1f532, 0x1f549, LBC::al}, {0x1f54a, 0x1f573, LBC::id}, {0x1f574, 0x1f575, LBC::eb},
	{0x1f576, 0x1f579, LBC::id}, {0x1f57a, 0x1f57a, LBC::eb}, {0x1f57b, 0x1f58f, LBC::id}, {0x1f590, 0x1f590, LBC::eb},
	{0x1f591, 0x1f594, LBC::id}, {0x1f595, 0x1f596, LBC::eb}, {0x1f597, 0x1f5d3, LBC::id}, {0x1f5d4, 0x1f5db, LBC::al},
	{0x1f5dc, 0x1f5f3, LBC::id}, {0x1f5f4, 0x1f5f9, LBC::al}, {0x1f5fa, 0x1f644, LBC::id}, {0x1f645, 0x1f647, LBC::eb},
	{0x1f648, 0x1f64a, LBC::id}, {0x1f64b, 0x1f64f, LBC::eb}, {0x1f650, 0x1f675, LBC::al}, {0x1f676, 0x1f678, LBC::qu},
	{0x1f679, 0x1f67b, LBC::ns}, {0x1f67c, 0x1f67f, LBC::al}, {0x1f680, 0x1f6a2, LBC::id}, {0x1f6a3, 0x1f6a3, LBC::eb},
	{0x1f6a4, 0x1f6b3, LBC::id}, {0x1f6b4, 0x1f6b6, LBC::eb}, {0x1f6b7, 0x1f6bf, LBC::id}, {0x1f6c0, 0x1f6c0, LBC::eb},
	{0x1f6c1, 0x1f6cb, LBC::id}, {0x1f6cc, 0x1f6cc, LBC::eb}, {0x1f6cd, 0x1f6ff, LBC::id}, {0x1f700, 0x1f773, LBC::al},
	{0x1f774, 0x1f776, LBC::id}, {0x1f777, 0x1f77a, LBC::al}, {0x1f77b, 0x1f77f, LBC::id}, {0x1f780, 0x1f7d4, LBC::al},
	{0x1f7d5, 0x1f7da, LBC::id}, {0x1f7db, 0x1f7db, LBC::al}, {0x1f7dc, 0x1f7f0, LBC::id}, {0x1f7f1, 0x1f80b, LBC::al},
	{0x1f810, 0x1f847, LBC::al}, {0x1f850, 0x1f859, LBC::al}, {0x1f860, 0x1f887, LBC::al}, {0x1f890, 0x1f8ad, LBC::al},
	{0x1f8b0, 0x1f8bb, LBC::al}, {0x1f8c0, 0x1f8c1, LBC::al}, {0x1f8d0, 0x1f8d8, LBC::al}, {0x1f900, 0x1f90b, LBC::al},
	{0x1f90c, 0x1f90c, LBC::eb}, {0x1f90d, 0x1f90e, LBC::id}, {0x1f90f, 0x1f90f, LBC::eb}, {0x1f910, 0x1f917, LBC::id},
	{0x1f918, 0x1f91f, LBC::eb}, {0x1f920, 0x1f925, LBC::id}, {0x1f926, 0x1f926, LBC::eb}, {0x1f927, 0x1f92f, LBC::id},
	{0x1f930, 0x1f939, LBC::eb}, {0x1f93a, 0x1f93b, LBC::id}, {0x1f93c, 0x1f93e, LBC::eb}, {0x1f93f, 0x1f976, LBC::id},
	{0x1f977, 0x1f977, LBC::eb}, {0x1f978, 0x1f9b4, LBC::id}, {0x1f9b5, 0x1f9b6, LBC::eb}, {0x1f9b7, 0x1f9b7, LBC::id},
	{0x1f9b8, 0x1f9b9, LBC::eb}, {0x1f9ba, 0x1f9ba, LBC::id}, {0x1f9bb, 0x1f9bb, LBC::eb}, {0x1f9bc, 0x1f9cc, LBC::id},
	{0x1f9cd, 0x1f9cf, LBC::eb}, {0x1f9d0, 0x1f9d0, LBC::id}, {0x1f9d1, 0x1f9dd, LBC::eb}, {0x1f9de, 0x1f9ff, LBC::id},
	{0x1fa00, 0x1fa57, LBC::al}, {0x1fa58, 0x1fac2, LBC::id}, {0x1fac3, 0x1fac5, LBC::eb}, {0x1fac6, 0x1faef, LBC::id},
	{0x1faf0, 0x1fafa, LBC::eb}, {0x1fafb, 0x1faff, LBC::id}, {0x1fb00, 0x1fb92, LBC::al}, {0x1fb94, 0x1fbef, LBC::al},
	{0x1fbf0, 0x1fbf9, LBC::nu}, {0x1fbfa, 0x1fbfa, LBC::al}, {0x1fc00, 0x1fffd, LBC::id}, {0x20000, 0x2fffd, LBC::id},
	{0x30000, 0x3fffd, LBC::id}, {0xe0001, 0xe0001, LBC::cm}, {0xe0020, 0xe007f, LBC::cm}, {0xe0100, 0xe01ef, LBC::cm},
	{0xf0000, 0xffffd, LBC::xx}, {0x100000, 0x10fffd, LBC::xx}
};

#endif


//
//	lineBreakRangeFind
//

template <typename T, typename C>
LBC lineBreakRangeFind(const T& table, C codepoint) {
	auto low = std::begin(table);
	auto high = std::end(table);

	while (low <= high) {
		auto mid = low + (high - low) / 2;

		if (codepoint >= mid->low && codepoint <= mid->high) {
			return mid->lbc;

		} else if (codepoint < mid->low) {
			high = mid - 1;

		} else {
			low = mid + 1;
		}
	}

	return LBC::undefined;
}


//
//	getLineBreakClass
//

static LBC getLineBreakClass(ImWchar codepoint) {
	LBC lbc;

	// handle simple 8-bit ASCII codepoints
	if (codepoint < 256) {
		return lineBreak8[codepoint];
	}

#if defined(IMGUI_USE_WCHAR32)
	if (codepoint >= 0x10000) {
		lbc = lineBreakRangeFind(lineBreak32, static_cast<char32_t>(codepoint));

	} else
#endif

	{
		lbc = lineBreakRangeFind(lineBreak16, static_cast<char16_t>(codepoint));
	}

	if (lbc == LBC::undefined) {
		if ((codepoint >= 0x3400 && codepoint <= 0x4DBF) ||
			(codepoint >= 0x4E00 && codepoint <= 0x9FFF) ||
			(codepoint >= 0xF900 && codepoint <= 0xFAFF)

#if defined(IMGUI_USE_WCHAR32)
			||
			(codepoint >= 0x20000 && codepoint <= 0x2FFFD) ||
			(codepoint >= 0x30000 && codepoint <= 0x3FFFD) ||

			(codepoint >= 0x1F000 && codepoint <= 0x1FAFF) ||
			(codepoint >= 0x1FC00 && codepoint <= 0x1FFFD)
#endif
		) {

			lbc = LBC::id;

		} else if (codepoint >= 0x20A0 && codepoint <= 0x20CF) {
			lbc = LBC::pr;

		} else {
			lbc = LBC::xx;
		}
	}

	// LB1: assign a line breaking class to each code point of the input
	// resolve AI, CB, CJ, SA, SG, and XX into other line breaking classes
	if (lbc == LBC::ai || lbc == LBC::sg || lbc == LBC::xx) {
		lbc = LBC::al;

	} else if (lbc == LBC::sa) {
		lbc = LBC::al;

	} else if (lbc == LBC::cj) {
		lbc = LBC::ns;
	}

	return lbc;
}


//
//	State machine status
//

static constexpr size_t invalidPos = std::numeric_limits<size_t>::max();
static constexpr ImWchar dotCircle = 0x25CC;

struct LineBreakGlyph {
	LineBreakGlyph() = default;
	LineBreakGlyph(ImWchar codepoint, LBC cls, size_t pos) : codepoint(codepoint), cls(cls), pos(pos) {}
	ImWchar codepoint = 0;
	LBC cls = LBC::sot;
	size_t pos = invalidPos;
	bool ignored = false;
};

struct LineBreakState {
	TextEditor::Glyph* glyphs;
	size_t size;
	LineBreakGlyph previous;
	LineBreakGlyph current;
	LineBreakGlyph next;
	bool spaces = false;
	bool lb8 = false;
	size_t ri = 0;

	// move to the next state
	inline void push(const LineBreakGlyph& step) {
		if (next.ignored) {
			current.pos = next.pos;

		} else {
			previous = current;
			current = next;
		}

		next = step;
	}

	// get the codepoint at specified location
	ImWchar getCodepoint(size_t pos) const {
		if (pos < size) {
			return glyphs[pos].codepoint;

		} else {
			return 0;
		}
	}

	// get the line break class at specified location
	LBC getClass(size_t pos) const {
		if (pos < size) {
			return getLineBreakClass(glyphs[pos].codepoint);

		} else {
			return LBC::eot;
		}
	}
};


//
//	Support functions
//

static inline bool isPf(ImWchar codepoint) {
	if (codepoint < 0x00bb) {
		return false;

	} else {
		static std::unordered_set<ImWchar> Pf = {
			0x00bb, 0x2019, 0x201d, 0x203a, 0x2e03, 0x2e05, 0x2e0a, 0x2e0d, 0x2e1d, 0x2e21
		};

		return Pf.find(codepoint) != Pf.end();
	}
}


static inline bool isPi(ImWchar codepoint) {
	if (codepoint < 0x00ab) {
		return false;

	} else {
		static std::unordered_set<ImWchar> Pi = {
			0x00ab, 0x2018, 0x201b, 0x201c, 0x201f, 0x2039, 0x2e02, 0x2e04, 0x2e09, 0x2e0c, 0x2e1c, 0x2e20
		};

		return Pi.find(codepoint) != Pi.end();
	}
}


static inline bool isAkCircleAs(const LineBreakGlyph& lbg) {
	return (lbg.cls == LBC::ak) || (lbg.codepoint == 0x25CC) || (lbg.cls == LBC::as);
}


//
//	Line break rules
//
//	Partly ported from on https://github.com/cto-af/linebreak
//

static inline TextEditor::BreakOption lb2(const LineBreakState& state) {
	// LB2: never break at the start of text
	// sot ×
	if (state.current.cls == LBC::sot && state.next.cls != LBC::eot) {
		return TextEditor::BreakOption::noBreak;

	} else {
		return TextEditor::BreakOption::undefined;
	}
}


static inline TextEditor::BreakOption lb3(const LineBreakState& state) {
	// LB3: always break at the end of text
	// ! eot
	if (state.next.cls == LBC::eot) {
		return TextEditor::BreakOption::mustBreak;

	} else {
		return TextEditor::BreakOption::undefined;
	}
}


static inline TextEditor::BreakOption lb4(const LineBreakState& state) {
	// LB4: always break after hard line breaks
	// BK !
	if (state.current.cls == LBC::bk) {
		return TextEditor::BreakOption::mustBreak;

	} else {
		return TextEditor::BreakOption::undefined;
	}
}


static inline TextEditor::BreakOption lb5(const LineBreakState& state) {
	// LB5: treat CR followed by LF, as well as CR, LF, and NL as hard line breaks
	// CR × LF
	// CR !
	// LF !
	// NL !
	switch (state.current.cls) {
		case LBC::cr:
			if (state.next.cls == LBC::lf) {
				return TextEditor::BreakOption::noBreak;
			}

			return TextEditor::BreakOption::mustBreak;

		case LBC::lf:
		case LBC::nl:
			return TextEditor::BreakOption::mustBreak;

		default:
			return TextEditor::BreakOption::undefined;
	}
}


static inline TextEditor::BreakOption lb6(const LineBreakState& state) {
	// LB6: do not break before hard line breaks
	// × ( BK | CR | LF | NL )
	switch (state.next.cls) {
		case LBC::bk:
		case LBC::cr:
		case LBC::lf:
		case LBC::nl:
			return TextEditor::BreakOption::noBreak;

		default:
			return TextEditor::BreakOption::undefined;
	}
}

static inline TextEditor::BreakOption lbSpaceStop(LineBreakState& state) {
	// for rules that have "do not break within even with intervening spaces", such as LB15
	if (state.current.cls != LBC::ri) {
		state.ri = 0;
	}

	if (state.spaces) {
		if (state.next.cls != LBC::sp) {
			state.spaces = false;
		}

		return TextEditor::BreakOption::noBreak;
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb7(const LineBreakState& state) {
	// LB7: do not break before spaces or zero width space
	// × ZW
	if (state.next.cls == LBC::zw) {
		return TextEditor::BreakOption::noBreak;
	}

	// × SP
	if (state.next.cls == LBC::sp) {
		switch (state.current.cls) {
			case LBC::zw: // see LB8
			case LBC::op: // see LB14
			case LBC::qu: // see LB15
			case LBC::cl: // see LB16
			case LBC::cp: // see LB16
			case LBC::b2: // see LB17
				break;

			default:
				return TextEditor::BreakOption::noBreak;
		}
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb8(LineBreakState& state) {
	// LB8: break before any character following a zero-width space, even if one or more spaces intervene
	// ZW SP* ÷
	if (state.lb8) {
		state.lb8 = false;
		return TextEditor::BreakOption::allowBreak;

	} else if (state.current.cls == LBC::zw) {
		if (state.next.cls == LBC::sp) {
			state.lb8 = true;
			return TextEditor::BreakOption::noBreak;
		}

		return TextEditor::BreakOption::allowBreak;
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb8a(const LineBreakState& state) {
	// LB8a: do not break after a zero width joiner
	// ZWJ ×
	if (state.current.cls == LBC::zwj) {
		return TextEditor::BreakOption::noBreak;

	} else {
		return TextEditor::BreakOption::undefined;
	}
}


static inline TextEditor::BreakOption lb9(LineBreakState& state) {
	// LB9: do not break a combining character sequence; treat it as if it has the
	// line breaking class of the base character in all of the following rules
	static const std::unordered_set<LBC> BKCRLFNLSPZW = {LBC::bk, LBC::cr, LBC::lf, LBC::nl, LBC::sp, LBC::zw};

	// treat X (CM | ZWJ)* as if it were X
	// where X is any line break class except BK, CR, LF, NL, SP, or ZW
	if (BKCRLFNLSPZW.find(state.current.cls) == BKCRLFNLSPZW.end() &&
		(state.next.cls == LBC::cm || state.next.cls == LBC::zwj)) {

		state.next.ignored = true;
		return TextEditor::BreakOption::noBreak;
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb10(LineBreakState& state) {
	// LB10: treat any remaining combining mark or ZWJ as AL
	if (state.current.cls == LBC::cm) {
		state.current.cls = LBC::al;
	}

	if (state.next.cls == LBC::cm) {
		state.next.cls = LBC::al;
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb11(const LineBreakState& state) {
	// LB11: do not break before or after word joiner and related characters
	// × WJ
	// WJ ×
	if (state.next.cls == LBC::wj || state.current.cls == LBC::wj) {
		return TextEditor::BreakOption::noBreak;

	} else {
		return TextEditor::BreakOption::undefined;
	}
}


static inline TextEditor::BreakOption lb12(const LineBreakState& state) {
	// LB12: do not break after NBSP and related characters
	// GL ×
	if (state.current.cls == LBC::gl) {
		return TextEditor::BreakOption::noBreak;

	} else {
		return TextEditor::BreakOption::undefined;
	}
}


static inline TextEditor::BreakOption lb12a(const LineBreakState& state) {
	// LB12a: do not break before NBSP and related characters, except after spaces and hyphens
	// [^SP BA HY HH] × GL
	if (state.next.cls == LBC::gl) {
		switch (state.current.cls) {
			case LBC::sp:
			case LBC::ba:
			case LBC::hy:
			case LBC::hh:
				return TextEditor::BreakOption::undefined;

			default:
				return TextEditor::BreakOption::noBreak;
		}
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb13(const LineBreakState& state) {
	// LB13: do not break before ‘]’ or ‘!’ or ‘;’ or ‘/’, even after spaces
	// × CL
	// × CP
	// × EX
	// × SY
	switch (state.next.cls) {
		case LBC::cl:
		case LBC::cp:
		case LBC::ex:
		case LBC::sy:
			return TextEditor::BreakOption::noBreak;

		default:
			return TextEditor::BreakOption::undefined;
	}
}


static inline TextEditor::BreakOption lb14(LineBreakState& state) {
	// LB14: do not break after ‘[’, even after spaces
	// OP SP* ×
	if (state.current.cls == LBC::op) {
		if (state.next.cls == LBC::sp) {
			state.spaces = true;
		}

		return TextEditor::BreakOption::noBreak;
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb15a(LineBreakState& state) {
	// LB15a: do not break after an unresolved initial punctuation that lies at the start of the line,
	// after a space, after opening punctuation, or after an unresolved quotation mark, even after spaces
	static const std::unordered_set<LBC> sotBKCRLFNLOPQUGLSPZW = {
		LBC::sot, LBC::bk, LBC::cr, LBC::lf, LBC::nl, LBC::op, LBC::qu, LBC::gl, LBC::sp, LBC::zw
	};

	// (sot | BK | CR | LF | NL | OP | QU | GL | SP | ZW) [\p{Pi}&QU] SP* ×
	if (sotBKCRLFNLOPQUGLSPZW.find(state.previous.cls) != sotBKCRLFNLOPQUGLSPZW.end() &&
		isPi(state.current.codepoint) &&
		state.current.cls == LBC::qu) {

		state.spaces = true;
		return TextEditor::BreakOption::noBreak;
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb15b(const LineBreakState& state) {
	// LB15b: do not break before an unresolved final punctuation that lies at the end of the line,
	// before a space, before a prohibited break, or before an unresolved quotation mark, even after spaces
	static const std::unordered_set<LBC> SPGLWJCLQUCPEXISSYBKCRLFNLZW = {
		LBC::sp, LBC::gl, LBC::wj, LBC::cl, LBC::qu, LBC::cp, LBC::ex,
		LBC::is, LBC::sy, LBC::bk, LBC::cr, LBC::lf, LBC::nl, LBC::zw
	};

	// × [\p{Pf}&QU] ( SP | GL | WJ | CL | QU | CP | EX | IS | SY | BK | CR | LF | NL | ZW | eot)
	if (isPf(state.next.codepoint) && state.next.cls == LBC::qu) {
		const auto after = state.getClass(state.next.pos + 1);

		if (after == LBC::eot) {
			return TextEditor::BreakOption::noBreak;
		}

		if (SPGLWJCLQUCPEXISSYBKCRLFNLZW.find(after) == SPGLWJCLQUCPEXISSYBKCRLFNLZW.end()) {
			return TextEditor::BreakOption::noBreak;
		}
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb15c(const LineBreakState& state) {
	// LB15c: Break before a decimal mark that follows a space, for instance, in ‘subtract .5’
	// SP ÷ IS NU
	if ((state.current.cls == LBC::sp) && (state.next.cls == LBC::is)) {
		if (state.getClass(state.next.pos + 1) == LBC::nu) {
			return TextEditor::BreakOption::allowBreak;
		}
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb15d(const LineBreakState& state) {
	// LB15d: otherwise, do not break before ‘;’, ‘,’, or ‘.’, even after spaces
	// × IS
	if (state.next.cls == LBC::is) {
		return TextEditor::BreakOption::noBreak;
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb16(LineBreakState& state) {
	// LB16: do not break between closing punctuation and a nonstarter (lb=NS), even with intervening space
	// (CL | CP) SP* × NS
	if ((state.current.cls == LBC::cl) || (state.current.cls == LBC::cp)) {
		auto pos = state.next.pos;
		LBC cls;

		do {
			cls = state.getClass(pos++);
		} while (cls == LBC::sp);

		if (state.getClass(pos) == LBC::ns) {
			if (state.next.cls == LBC::sp) {
				state.spaces = true;
			}

			return TextEditor::BreakOption::noBreak;
		}

		if (state.next.cls == LBC::sp) {
			return TextEditor::BreakOption::noBreak;
		}
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb17(LineBreakState& state) {
	// LB17: do not break within ‘——’, even with intervening spaces
	// B2 SP* × B2
	if (state.current.cls == LBC::b2) {
		auto pos = state.next.pos;
		LBC cls;

		do {
			cls = state.getClass(pos++);
		} while (cls == LBC::sp);

		if (state.getClass(pos) == LBC::b2) {
			if (state.next.cls != LBC::sp) {
				return TextEditor::BreakOption::noBreak;
			}

			state.spaces = true;
			return TextEditor::BreakOption::noBreak;

		} else if (state.next.cls == LBC::sp) {
			return TextEditor::BreakOption::noBreak;
		}
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb18(const LineBreakState& state) {
	// LB18: Break after spaces
	// SP ÷
	if (state.current.cls == LBC::sp) {
		return TextEditor::BreakOption::allowBreak;

	} else {
		return TextEditor::BreakOption::undefined;
	}
}

static inline TextEditor::BreakOption lb19(const LineBreakState& state) {
	// LB19: do not break before non-initial unresolved quotation marks, such as ‘ ” ’ or ‘ " ’,
	// nor after non-final unresolved quotation marks, such as ‘ “ ’ ‘ " ’
	// × [ QU - \p{Pi} ]
	if ((state.next.cls == LBC::qu) && !isPi(state.next.codepoint)) {
		// Gc=Pi is initial punctuation
		return TextEditor::BreakOption::noBreak;
	}

	// [ QU - \p{Pf} ] ×
	if ((state.current.cls == LBC::qu) && !isPf(state.current.codepoint)) {
		// Gc=Pf is final punctuation
		return TextEditor::BreakOption::noBreak;
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb19a(const LineBreakState& state) {
	// LB19a: unless surrounded by East Asian characters, do not break either side
	// [^$EastAsian] × QU
	if (!TextEditor::CodePoint::isEastAsian(state.current.codepoint) && (state.next.cls == LBC::qu)) {
		return TextEditor::BreakOption::noBreak;
	}

	// × QU ( [^$EastAsian] | eot )
	if (state.next.cls == LBC::qu) {
		const auto afterCodepoint = state.getCodepoint(state.next.pos + 1);
		const auto afterCls = state.getClass(state.next.pos + 1);

		if (afterCls == LBC::eot || !TextEditor::CodePoint::isEastAsian(afterCodepoint)) {
			return TextEditor::BreakOption::noBreak;
		}
	}

	// QU × [^$EastAsian]
	if ((state.current.cls == LBC::qu) && !TextEditor::CodePoint::isEastAsian(state.next.codepoint)) {
		return TextEditor::BreakOption::noBreak;
	}

	// ( sot | [^$EastAsian] ) qu ×
	if ((state.previous.cls == LBC::sot || !TextEditor::CodePoint::isEastAsian(state.previous.codepoint)) && state.current.cls == LBC::qu) {
		return TextEditor::BreakOption::noBreak;
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb20(const LineBreakState& state) {
	// LB20: break before and after unresolved CB
	// ÷ CB
	// CB ÷
	if ((state.current.cls == LBC::cb) || (state.next.cls == LBC::cb)) {
		return TextEditor::BreakOption::allowBreak;

	} else {
		return TextEditor::BreakOption::undefined;
	}
}


static inline TextEditor::BreakOption lb20a(const LineBreakState& state) {
	// LB20a: do not break after a word-initial hyphen
	static const std::unordered_set<LBC> sotBKCRLFNLSPZWCBGL = {
		LBC::sot, LBC::bk, LBC::cr, LBC::lf, LBC::nl, LBC::sp, LBC::zw, LBC::cb, LBC::gl
	};

	// ( sot | BK | CR | LF | NL | SP | ZW | CB | GL ) ( HY | HH ) × ( AL | HL )
	if (sotBKCRLFNLSPZWCBGL.find(state.previous.cls) != sotBKCRLFNLSPZWCBGL.end() &&
		((state.current.cls == LBC::hy) || (state.current.cls == LBC::hh)) &&
		((state.next.cls == LBC::al) || (state.next.cls == LBC::hl))) {

		return TextEditor::BreakOption::noBreak;

	} else {
		return TextEditor::BreakOption::undefined;
	}
}


static inline TextEditor::BreakOption lb21(const LineBreakState& state) {
	// LB21: do not break before hyphen-minus, other hyphens, fixed-width spaces, small kana, and other non-starters, or after acute accents
	// BB ×
	if (state.current.cls == LBC::bb) {
		return TextEditor::BreakOption::noBreak;
	}

	// × ( BA | HH | HY | NS )
	switch (state.next.cls) {
		case LBC::ba:
		case LBC::hh:
		case LBC::hy:
		case LBC::ns:
			return TextEditor::BreakOption::noBreak;

		default:
			return TextEditor::BreakOption::undefined;
	}
}


static inline TextEditor::BreakOption lb21a(const LineBreakState& state) {
	// LB21a: do not break after the hyphen in Hebrew + Hyphen + non-Hebrew
	// HL (HY | HH) × [^HL]
	if ((state.previous.cls == LBC::hl) &&
		(state.current.cls == LBC::hy || state.current.cls == LBC::hh) &&
		state.next.cls != LBC::hl) {

		return TextEditor::BreakOption::noBreak;

	} else {
		return TextEditor::BreakOption::undefined;
	}
}


static inline TextEditor::BreakOption lb21b(const LineBreakState& state) {
	// don’t break between Solidus and Hebrew letters
	// SY × HL
	if ((state.current.cls == LBC::sy) && (state.next.cls == LBC::hl)) {
		return TextEditor::BreakOption::noBreak;

	} else {
		return TextEditor::BreakOption::undefined;
	}
}


static inline TextEditor::BreakOption lb22(const LineBreakState& state) {
	// do not break before ellipses
	// × IN
	if (state.next.cls == LBC::in) {
		return TextEditor::BreakOption::noBreak;

	} else {
		return TextEditor::BreakOption::undefined;
	}
}


static inline TextEditor::BreakOption lb23(const LineBreakState& state) {
	// do not break between digits and letters
	switch (state.current.cls) {
		case LBC::al:
		case LBC::hl:
			// (AL | HL) × NU
			if (state.next.cls == LBC::nu) {
				return TextEditor::BreakOption::noBreak;
			}

			break;

		case LBC::nu:
			// NU × (AL | HL)
			if ((state.next.cls == LBC::al) || (state.next.cls == LBC::hl)) {
				return TextEditor::BreakOption::noBreak;
			}

			break;

		default:
			return TextEditor::BreakOption::undefined;
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb23a(const LineBreakState& state) {
	// LB23a: do not break between numeric prefixes and ideographs, or between ideographs and numeric postfixes
	static const std::unordered_set<LBC> IDEBEM = {LBC::id, LBC::eb, LBC::em};

	// PR × (ID | EB | EM)
	if ((state.current.cls == LBC::pr) && IDEBEM.find(state.next.cls) != IDEBEM.end()) {
		return TextEditor::BreakOption::noBreak;
	}

	// (ID | EB | EM) × PO
	if ((state.next.cls == LBC::po) && IDEBEM.find(state.current.cls) != IDEBEM.end()) {
		return TextEditor::BreakOption::noBreak;
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb24(const LineBreakState& state) {
	// LB24: do not break between numeric prefix/postfix and letters, or between letters and prefix/postfix
	// (PR | PO) × (AL | HL)
	if ((state.current.cls == LBC::pr || state.current.cls == LBC::po) &&
		(state.next.cls == LBC::al || state.next.cls == LBC::hl)) {
		return TextEditor::BreakOption::noBreak;
	}

	// (AL | HL) × (PR | PO)
	if ((state.current.cls == LBC::al || state.current.cls == LBC::hl) &&
		(state.next.cls == LBC::pr || state.next.cls == LBC::po)) {
		return TextEditor::BreakOption::noBreak;
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb25(const LineBreakState& state) {
	// LB25: do not break numbers
	// approach: find the end of a matching run, then no-break everything as we go past it
	static const std::unordered_set<LBC> POPR = {LBC::po, LBC::pr};
	static const std::unordered_set<LBC> CLCP = {LBC::cl, LBC::cp};

	// NU ( SY | IS )* CL × PO
	// NU ( SY | IS )* CP × PO
	// NU ( SY | IS )* CL × PR
	// NU ( SY | IS )* CP × PR
	// NU ( SY | IS )* × PO
	// NU ( SY | IS )* × PR
	// NU ( SY | IS )* × NU
	size_t pos = invalidPos;

	if (POPR.find(state.next.cls) != POPR.end()) {
		if (CLCP.find(state.current.cls) != CLCP.end()) {
			pos = state.previous.pos;

		} else {
			pos = state.current.pos;
		}

	} else if (state.next.cls == LBC::nu) {
		pos = state.current.pos;
	}

	// as specified, possible ReDoS because of the backtracking
	if (pos != invalidPos) {
		while (pos != invalidPos) {
			switch(state.getClass(pos)) {
				case LBC::sy:
				case LBC::is:
					break;

				case LBC::nu:
					return TextEditor::BreakOption::noBreak;

				default:
					pos = 0;
			}

			pos = pos == 0 ? invalidPos : pos - 1;
		}
	}

	// PO × OP NU
	// PO × OP IS NU
	// PO × NU
	// PR × OP NU
	// PR × OP IS NU
	// PR × NU
	if ((state.current.cls == LBC::po) || ((state.current.cls == LBC::pr))) {
		if (state.next.cls == LBC::op) {
			const auto after = state.getClass(state.next.pos + 1);

			if (after != LBC::eot) {
				if (after == LBC::nu) {
					return TextEditor::BreakOption::noBreak;

				} else if (after == LBC::is) {
					const auto afterAfter = state.getClass(state.next.pos + 2);

					if (afterAfter == LBC::nu) {
						return TextEditor::BreakOption::noBreak;
					}
				}
			}

		} else if (state.next.cls == LBC::nu) {
			return TextEditor::BreakOption::noBreak;
		}
	}

	// HY × NU
	if ((state.current.cls == LBC::hy) && (state.next.cls == LBC::nu)) {
		return TextEditor::BreakOption::noBreak;
	}

	// IS × NU
	if ((state.current.cls == LBC::is) && (state.next.cls == LBC::nu)) {
		return TextEditor::BreakOption::noBreak;
	}

		return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb26(const LineBreakState& state) {
	// LB26 do not break a Korean syllable
	static const std::unordered_set<LBC> JLJVH2H3 = {LBC::jl, LBC::jv, LBC::h2, LBC::h3};
	static const std::unordered_set<LBC> JVJT = {LBC::jv, LBC::jt};

	switch (state.current.cls) {
		case LBC::jl:
			// JL × (JL | JV | H2 | H3)
			if (JLJVH2H3.find(state.next.cls) != JLJVH2H3.end()) {
				return TextEditor::BreakOption::noBreak;
			}

			break;

		case LBC::jv:
		case LBC::h2:
			// (JV | H2) × (JV | JT)
			if (JVJT.find(state.next.cls) != JVJT.end()) {
				return TextEditor::BreakOption::noBreak;
			}

			break;

		case LBC::jt:
		case LBC::h3:
			// (JT | H3) × JT
			if (state.next.cls == LBC::jt) {
				return TextEditor::BreakOption::noBreak;
			}

			break;

		default:
			return TextEditor::BreakOption::undefined;
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb27(const LineBreakState& state) {
	// LB27: treat a Korean Syllable Block the same as LBC::id
	static const std::unordered_set<LBC> JLJVJTH2H3 = {LBC::jl, LBC::jv, LBC::jt, LBC::h2, LBC::h3};

	switch (state.current.cls) {
		case LBC::jl:
		case LBC::jv:
		case LBC::jt:
		case LBC::h2:
		case LBC::h3:
			// (JL | JV | JT | H2 | H3) × PO
			if (state.next.cls == LBC::po) {
				return TextEditor::BreakOption::noBreak;
			}

			break;

		case LBC::pr:
			// PR × (JL | JV | JT | H2 | H3)
			if (JLJVJTH2H3.find(state.next.cls) != JLJVJTH2H3.end()) {
				return TextEditor::BreakOption::noBreak;
			}

			break;

		default:
			return TextEditor::BreakOption::undefined;
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb28(const LineBreakState& state) {
	// LB28 do not break between alphabetics (“at”)
	// (AL | HL) × (AL | HL)
	if ((state.current.cls == LBC::al || state.current.cls == LBC::hl) &&
		(state.next.cls == LBC::al || state.next.cls == LBC::hl)) {

		return TextEditor::BreakOption::noBreak;

	} else {
		return TextEditor::BreakOption::undefined;
	}
}


static inline TextEditor::BreakOption lb28a(const LineBreakState& state) {
	// LB28a: do not break inside the orthographic syllables of Brahmic scripts
	// AP × (AK | [◌] | AS)
	if ((state.current.cls == LBC::ap) && isAkCircleAs(state.next)) {
		return TextEditor::BreakOption::noBreak;
	}

	// (AK | [◌] | AS) × (VF | VI)
	if (isAkCircleAs(state.current) && ((state.next.cls == LBC::vf) || (state.next.cls == LBC::vi))) {
		return TextEditor::BreakOption::noBreak;
	}

	// (AK | [◌] | AS) VI × (AK | [◌])
	if (isAkCircleAs(state.previous) &&
		state.current.cls == LBC::vi && (state.next.cls == LBC::ak || state.next.codepoint == dotCircle)) {

		return TextEditor::BreakOption::noBreak;
	}

	// (AK | [◌] | AS) × (AK | [◌] | AS) VF
	if (isAkCircleAs(state.current) &&
		isAkCircleAs(state.next) && (state.getClass(state.next.pos + 1) == LBC::vf)) {

		return TextEditor::BreakOption::noBreak;
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb29(const LineBreakState& state) {
	// LB29: do not break between numeric punctuation and alphabetics (“e.g.”)
	// IS × (AL | HL)
	if (state.current.cls == LBC::is && (state.next.cls == LBC::al || state.next.cls == LBC::hl)) {
		return TextEditor::BreakOption::noBreak;

	} else {
		return TextEditor::BreakOption::undefined;
	}
}


static inline TextEditor::BreakOption lb30(const LineBreakState& state) {
	// LB30: do not break between letters, numbers, or ordinary symbols and opening or closing parentheses
	static const std::unordered_set<LBC> ALHLNU = {LBC::al, LBC::hl, LBC::nu};

	switch (state.current.cls) {
		case LBC::al:
		case LBC::hl:
		case LBC::nu:
			// (AL | HL | NU) × [OP-$EastAsian]
			if (state.next.cls == LBC::op && !TextEditor::CodePoint::isEastAsian(state.next.codepoint)) {
				return TextEditor::BreakOption::noBreak;
			}

			break;

		case LBC::cp:
			// [CP-$EastAsian] × (AL | HL | NU)
			if (!TextEditor::CodePoint::isEastAsian(state.current.codepoint) &&
				ALHLNU.find(state.next.cls) != ALHLNU.end()) {

				return TextEditor::BreakOption::noBreak;
			}

			break;

		default:
			return TextEditor::BreakOption::undefined;
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb30a(LineBreakState& state) {
	// LB30a: break between two regional indicator symbols if and only if there are
	// an even number of regional indicators preceding the position of the break
	if (state.current.cls == LBC::ri) {
		if (state.next.cls == LBC::ri) {
			if (++state.ri % 2 != 0) {
				return TextEditor::BreakOption::noBreak;
			}
		}

	} else {
		state.ri = 0;
	}

	return TextEditor::BreakOption::undefined;
}


static inline TextEditor::BreakOption lb30b(const LineBreakState& state) {
	// LB30b: do not break between an emoji base (or potential emoji) and an emoji modifier
	// EB × EM
	if ((state.current.cls == LBC::eb) && (state.next.cls == LBC::em)) {
		return TextEditor::BreakOption::noBreak;
	}

#if defined(IMGUI_USE_WCHAR32)
	// [\p{Extended_Pictographic}&\p{Cn}] × EM
	// if (state.next.cls == LBC::em && (/^\p{ExtPict}$/u.test(state.cur.char))  && (/^\p{gc=Cn}$/u.test(state.cur.char))) {
	// 	return TextEditor::BreakOption::noBreak;
	// }
#endif

	return TextEditor::BreakOption::undefined;
}


#define RULE(r)														\
	result = r(state);												\
																	\
	if (result != TextEditor::BreakOption::undefined) {				\
		return result;												\
	}

#define RULE2(r)													\
	if (config.r) {													\
		RULE(r)														\
	}


static inline TextEditor::BreakOption applyRules(const TextEditor::LineBreakConfig& config, LineBreakState& state) {
	TextEditor::BreakOption result;
	RULE2(lb2);
	RULE2(lb3);
	RULE2(lb4);
	RULE2(lb5);
	RULE2(lb6);
	RULE(lbSpaceStop);
	RULE2(lb7);
	RULE2(lb8);
	RULE2(lb8a);
	RULE2(lb9);
	RULE2(lb10);
	RULE2(lb11);
	RULE2(lb12);
	RULE2(lb12a);
	RULE2(lb13);
	RULE2(lb14);
	RULE2(lb15a);
	RULE2(lb15b);
	RULE2(lb15c);
	RULE2(lb15d);
	RULE2(lb16);
	RULE2(lb17);
	RULE2(lb18);
	RULE2(lb19);
	RULE2(lb19a);
	RULE2(lb20);
	RULE2(lb20a);
	RULE2(lb21a);
	RULE2(lb21);
	RULE2(lb21b);
	RULE2(lb22);
	RULE2(lb23);
	RULE2(lb23a);
	RULE2(lb24);
	RULE2(lb25);
	RULE2(lb26);
	RULE2(lb27);
	RULE2(lb28);
	RULE2(lb28a);
	RULE2(lb29);
	RULE2(lb30);
	RULE2(lb30a);
	RULE2(lb30b);

	// LB31: break everywhere else
	// ALL ÷
	// ÷ ALL
	return TextEditor::BreakOption::allowBreak;
}


//
//	TextEditor::LineBreak::classify
//

void TextEditor::LineBreak::classify(Line& line) {
	// handle easy cases
	const auto size = line.size();

	if (size == 0) {
		return;

	} else if (size == 1) {
		line[0].breakOption = BreakOption::noBreak;
		return;
	}

	// handle different break modes
	if (config.useUnicodeAnnex14) {
		// setup state
		LineBreakState state;
		state.glyphs = &(*line.begin());
		state.size = size;

		for (size_t i = 0; i < size; i++) {
			const auto& glyph = line[i];
			state.push(LineBreakGlyph(glyph.codepoint, getLineBreakClass(glyph.codepoint), i));
			const auto breakOption = applyRules(config, state);

			if (i) {
				line[i - 1].breakOption = breakOption;
			}
		}

	} else {
		// update break sets (if required)
		if (updateSets) {
			updateSet(breakAfter, config.breakAfter);
			updateSet(breakBefore, config.breakBefore);
			updateSets = false;
		}

		// process all glyphs in line
		for (size_t i = 0; i < size; i++) {
			if (breakAfter.find(line[i].codepoint) != breakAfter.end()) {
				line[i].breakOption = BreakOption::allowBreak;

			} else if (i && breakBefore.find(line[i].codepoint) != breakBefore.end()) {
				line[i - 1].breakOption = BreakOption::allowBreak;
				line[i].breakOption = BreakOption::noBreak;

			} else {
				line[i].breakOption = BreakOption::noBreak;
			}
		}
	}

	line[size - 1].breakOption = BreakOption::noBreak;
}


//
//	TextEditor::LineBreak::updateSet
//

void TextEditor::LineBreak::updateSet(std::unordered_set<ImWchar>& set, std::string_view text) {
	// process UTF-8 and generate lines of glyphs
	set.clear();
	auto end = text.end();
	auto i = CodePoint::skipBOM(text.begin(), end);

	while (i < end) {
		ImWchar character;
		i = CodePoint::read(i, end, &character);
		set.insert(character);
	}
}
//	TextEditor::LineFold::update
//

bool TextEditor::LineFold::update(const Config& config, Document& document, const Bracketeer& bracketeer) {
	// force update after previous (un)fold request
	if (forceUpdate) {
		forceUpdate = false;
		updated = true;
	}

	// see if configuration changed
	if (lineFolding != config.lineFolding) {
		lineFolding = config.lineFolding;

		if (!lineFolding) {
			// line folding is turned off, clear list and reset all lines
			clear();

			for (auto& line : document) {
				line.foldingState = FoldingState::visible;
			}
		}

		updated = true;
	}

	// see if bracket layout changed
	if (bracketeer.isUpdated()) {
		updated = true;
	}

	// (re)build list of folding opportunities (if feature is on and updates have occurred)
	if (lineFolding && updated) {
		std::unordered_set<size_t> previouslyFolded;
		clear();

		for (auto& bracketPair : bracketeer) {
			// brackets must at least be one line apart
			if (bracketPair.end.line - bracketPair.start.line >= 1) {
				// we preserve old folding state
				if (document[bracketPair.start.line].foldingState == FoldingState::folded) {
					previouslyFolded.insert(bracketPair.start.line);
				}

				// add to list
				emplace_back(bracketPair.start.line, bracketPair.end.line);
			}
		}

		// apply new folding state to lines in document
		for (auto& line : document) {
			line.foldingState = FoldingState::visible;
		}

		for (const auto& fold : *this) {
			if (previouslyFolded.find(fold.start) != previouslyFolded.end()) {
				document[fold.start].foldingState = FoldingState::folded;

				for (size_t i = fold.start + 1; i <= fold.end; i++) {
					document[i].foldingState = FoldingState::hidden;
				}

			} else if (document[fold.start].foldingState != FoldingState::hidden) {
				document[fold.start].foldingState = FoldingState::foldable;
			}
		}
	}

	return updated;
}


//
//	TextEditor::LineFold::foldAroundLine
//

void TextEditor::LineFold::foldAroundLine(Document& document, size_t line) {
	auto lineToFold = invalidLine;

	for (const auto& fold : *this) {
		if (line > fold.start && line <= fold.end) {
			if (document[fold.start].foldingState == FoldingState::foldable) {
				lineToFold = fold.start;
			}
		}
	}

	if (lineToFold != invalidLine) {
		document[line].foldingState = FoldingState::folded;
		forceUpdate = true;
	}
}


//
//	TextEditor::LineFold::unfoldAroundLine
//

void TextEditor::LineFold::unfoldAroundLine(Document& document, size_t line) {
	for (const auto& fold : *this) {
		if (line > fold.start && line <= fold.end) {
			if (document[fold.start].foldingState == FoldingState::folded) {
				document[fold.start].foldingState = FoldingState::foldable;
				forceUpdate = true;
			}
		}
	}
}


//
//	TextEditor::LineFold::toggleAtLine
//

void TextEditor::LineFold::toggleAtLine(Document& document, size_t lineNo) {
	auto& line = document[lineNo];
	const auto state = line.foldingState;

	if (state == FoldingState::foldable) {
		line.foldingState = FoldingState::folded;

		for (const auto& fold : *this) {
			if (fold.start == lineNo) {
				for (size_t i = fold.start + 1; i <= fold.end; i++) {
					document[i].foldingState = FoldingState::hidden;
				}
			}
		}

		forceUpdate = true;

	} else if (state == FoldingState::folded) {
		line.foldingState = FoldingState::foldable;

		for (const auto& fold : *this) {
			if (fold.start == lineNo) {
				for (size_t i = fold.start + 1; i <= fold.end; i++) {
					document[i].foldingState = FoldingState::visible;
				}
			}
		}

		forceUpdate = true;
	}
}


//
//	TextEditor::LineFold::unfoldAll
//

void TextEditor::LineFold::unfoldAll(Document& document) {
	for (const auto& fold : *this) {
		if (document[fold.start].foldingState == FoldingState::folded) {
			document[fold.start].foldingState = FoldingState::foldable;
			forceUpdate = true;
		}
	}
}


//
//	TextEditor::TypeSetter::wrapLine
//

void TextEditor::TypeSetter::wrapLine(Line& line) {
	// there is always one visible row for a line
	line.rows = 1;

	// status for wrapped lines
	LineSections sections;

	// track breaks
	size_t lastBreakIndex = 0;
	size_t lastBreakableIndex = 0;
	size_t lastBreakableColumn = 0;
	size_t columns = 0;
	size_t indent = 0;

	// track first row indent
	bool isAtBeginning = true;
	bool isFirstRow = true;

	// process all glyphs on line
	size_t i = 0;
	const size_t size = line.size();

	while (i < size) {
		// handle hard break
		if (line[i].breakOption == BreakOption::mustBreak) {
			// this can't happen in the editor as hard breaks have already been handled
			// code is however left here in case we want to reuse it somewhere else later
			line.rows++;
			sections.emplace_back(lastBreakIndex, i, columns, indent);

			columns = 0;
			indent = 0;
			isAtBeginning = true;
			isFirstRow = true;
			lastBreakIndex = i + 1;
			lastBreakableIndex = i + 1;

		} else {
			// calculate first row indent (if required)
			if (isAtBeginning) {
				if (CodePoint::isWhiteSpace(line[i].codepoint)) {
					indent += line[i].columns;

				} else {
					isAtBeginning = false;
				}
			}

			// update column count
			columns += line[i].columns;

			if (columns < wordWrapColumns) {
				// we're not at the end of the row yet so we have to track any break options
				if (line[i].breakOption == BreakOption::allowBreak) {
					lastBreakableIndex = i + 1;
					lastBreakableColumn = columns;
				}

				i++;

			} else {
				// we have reached the end of the row
				if (lastBreakableIndex == lastBreakIndex) {
					// no breaks found, just truncate here
					lastBreakableIndex = i + 1;
					sections.emplace_back(lastBreakIndex, lastBreakableIndex, columns, isFirstRow ? 0 : indent);

				} else {
					// break at last breakable position
					sections.emplace_back(lastBreakIndex, lastBreakableIndex, lastBreakableColumn, isFirstRow ? 0 : indent);
				}

				i = lastBreakableIndex;

				// start a new row
				line.rows++;
				columns = indent;
				lastBreakIndex = lastBreakableIndex;
				lastBreakableColumn = indent;
				isFirstRow = false;
			}
		}
	}

	// did we end up with line wrapping?
	if (line.rows > 1) {
		// yes, save wrapping information
		sections.emplace_back(lastBreakIndex, size, columns, indent);
		line.sections = std::make_shared<LineSections>(sections);

		line.columns = 0;

		for (const auto& section : sections) {
			line.columns = std::max(line.columns, section.columns);
		}

	} else {
		// no wrapping
		line.sections = nullptr;
		line.columns = columns;
	}
}


//
//	TextEditor::TypeSetter::updateLine
//

void TextEditor::TypeSetter::updateLine(Line& line) {
	// update glyph widths
	size_t columns = 0;

	for (auto& glyph : line) {
		if (glyph.codepoint == '\t') {
			glyph.columns = static_cast<uint8_t>(tabSize - (columns % tabSize));

		} else {
			glyph.columns = static_cast<uint8_t>(CodePoint::getGlyphWidth(glyph.codepoint));
		}

		columns += glyph.columns;
	}

	if (wordWrap) {
		// classify all line break opportunities in line
		lineBreak.classify(line);

		// determine number of rows required
		wrapLine(line);

	} else {
		line.rows = 1;
		line.columns = columns;
		line.sections = nullptr;
	}

	line.needsTypeSetting = false;
}


//
//	TextEditor::TypeSetter::update
//

bool TextEditor::TypeSetter::update(const Config& config, Document& document, const LineFold& lineFold) {
	// see if the configuration changed
	const bool configChanged =
		tabSize != config.tabSize ||
		wordWrap != config.wordWrap ||
		(wordWrap && wordWrapColumns != config.wordWrapColumns);

	// update all lines on configuration changes
	if (configChanged) {
		// capture relevant configuration so we can detect new changes later
		tabSize = config.tabSize;
		wordWrap = config.wordWrap;
		wordWrapColumns = config.wordWrapColumns;

		// update all the lines
		for (auto& line : document) {
			updateLine(line);
		}

	// update changed lines when document is updated
	} else if (document.isUpdated()) {
		for (auto& line : document) {
			if (line.needsTypeSetting) {
				updateLine(line);
			}
		}
	}

	// update overall state if something changed
	updated = configChanged || document.isUpdated() || lineFold.isUpdated();

	if (updated) {
		// reset state
		clear();
		totalRows = 0;
		totalColumns = 0;

		// process all document lines
		size_t lineNo = 0;

		for (auto& line : document) {
			// update line's row start
			// this is even required for hidden lines so their DocPos can be transformed into a VisPos
			line.row = totalRows;

			// skip hidden lines
			if (line.foldingState != FoldingState::hidden) {
				// update total rows and columns
				totalRows += line.rows;
				totalColumns = std::max(totalColumns, line.columns);

				// add row(s) and handle word wrapping (if required)
				if (line.sections) {
					for (size_t i = 0; i < line.rows; i++) {
						emplace_back(lineNo, i, line.sections->at(i).columns);
					}

				} else {
					emplace_back(lineNo, 0, line.columns);
				}
			}

			lineNo++;
		}
	}

	// update and return status
	return updated;
}


//
//	TextEditor::TypeSetter::docPos2VisPos
//

TextEditor::VisPos TextEditor::TypeSetter::docPos2VisPos(const Document& document, DocPos pos) const {
	if (size() == 0) {
		return VisPos(0, 0);
	}

	auto& line = document[pos.line];
	VisPos visPos(line.row, 0);

	if (line.foldingState == FoldingState::hidden) {
		// for hidden lines, we use the position of the ellipsis (end of folded line)
		visPos.column = at(line.row).columns;

	} else if (line.sections) {
		// for wrapped lines, find relevant section and handle tabs
		bool done = false;

		for (size_t i = 0; !done && i < line.sections->size(); i++) {
			const auto& section = line.sections->at(i);

			if (pos.index >= section.startIndex && pos.index <= section.endIndex) {
				const auto start = line.begin() + section.startIndex;
				const auto end = line.begin() + pos.index;
				visPos.column = section.indent;

				for (auto glyph = start; glyph < end; glyph++) {
					visPos.column += glyph->columns;
				}

				done = true;

			} else {
				visPos.row++;
			}
		}

	} else {
		const auto end = line.begin() + pos.index;

		for (auto glyph = line.begin(); glyph < end; glyph++) {
			visPos.column += glyph->columns;
		}
	}

	return visPos;
}


//
//	TextEditor::TypeSetter::visPos2DocPos
//

TextEditor::DocPos TextEditor::TypeSetter::visPos2DocPos(const Document& document, VisPos pos) const {
	if (pos.row >= size()) {
		return DocPos(0, 0);
	}

	const auto& row = at(pos.row);
	const auto& line = document[row.line];

	DocPos docPos;
	docPos.line = row.line;

	size_t index;
	size_t leftColumn;
	size_t rightColumn;

	Line::const_iterator start;
	Line::const_iterator end;

	if (line.sections) {
		const auto& section = line.sections->at(row.section);
		index = section.startIndex;
		leftColumn = section.indent;
		rightColumn = section.indent;

		start = line.begin() + section.startIndex;
		end = line.begin() + section.endIndex;

	} else {
		index = 0;
		leftColumn = 0;
		rightColumn = 0;

		start = line.begin();
		end = line.end();
	}

	for (auto glyph = start; rightColumn < pos.column && glyph < end; glyph++) {
		leftColumn = rightColumn;
		rightColumn += glyph->columns;
		index++;
	}

	if (rightColumn - leftColumn <= 1) {
		docPos.index = index;

	} else {
		const auto leftDiff = pos.column - leftColumn;
		const auto rightDiff = rightColumn - pos.column;
		docPos.index = leftDiff <= rightDiff ? index - 1 : index;
	}

	return docPos;
}


//
//	TextEditor::TypeSetter::screenPos2DocPos
//

void TextEditor::TypeSetter::screenPos2DocPos(const Document& document, ImVec2 screenPos, DocPos& glyphPos, DocPos& cursorPos) const {
	// the returned glyphPos addresses the glyph pointed to by the screenPos parameter
	// the returned cursorPos returns the closest cursor position (which can be at the start or the end of the glyph)
	const size_t colNo = static_cast<size_t>(screenPos.x);
	const size_t rowNo = static_cast<size_t>(screenPos.y);

	if (screenPos.y <= 0.0f) {
		glyphPos = DocPos(0, 0);
		cursorPos = glyphPos;

	} else if (rowNo >= totalRows) {
		glyphPos = document.getBottom();
		cursorPos = glyphPos;

	} else {
		const auto& row = at(rowNo);
		const auto& line = document[row.line];

		if (screenPos.x <= 0.0f) {
			glyphPos = DocPos(row.line, 0);
			cursorPos = glyphPos;

		} else if (colNo > row.columns) {
			if (line.sections) {
				const auto& section = line.sections->at(row.section);
				glyphPos = DocPos(row.line, section.endIndex);

			} else {
				glyphPos = document.getEndOfLine(DocPos(row.line, 0));
			}

			cursorPos = glyphPos;

		} else {
			size_t index;
			size_t leftColumn;
			size_t rightColumn;

			Line::const_iterator start;
			Line::const_iterator end;

			if (line.sections) {
				const auto& section = line.sections->at(row.section);
				index = section.startIndex;
				leftColumn = section.indent;
				rightColumn = section.indent;

				start = line.begin() + section.startIndex;
				end = line.begin() + section.endIndex;

			} else {
				index = 0;
				leftColumn = 0;
				rightColumn = 0;

				start = line.begin();
				end = line.end();

			}

			for (auto glyph = start; static_cast<float>(rightColumn) < screenPos.x && glyph < end; glyph++) {
				leftColumn = rightColumn;
				rightColumn += glyph->columns;
				index++;
			}

			const auto leftDiff = screenPos.x - static_cast<float>(leftColumn);
			const auto rightDiff = static_cast<float>(rightColumn) - screenPos.x;

			glyphPos = DocPos(row.line, leftColumn == rightColumn ? index : index - 1);
			cursorPos = DocPos(row.line, leftDiff <= rightDiff ? index - 1 : index);
		}
	}
}


//
//	TextEditor::TypeSetter::normalizePos
//

TextEditor::VisPos TextEditor::TypeSetter::normalizePos(VisPos pos) const {
	if (totalRows == 0) {
		return VisPos(0, 0);

	} else if (pos.row >= totalRows) {
		const auto lastRow = totalRows - 1;
		return VisPos(lastRow, at(lastRow).columns);

	} else if (pos.column > at(pos.row).columns) {
		return VisPos(pos.row, at(pos.row).columns);

	} else {
		return pos;
	}
}


//
//	TextEditor::TypeSetter::isVisPosOverGlyph
//

bool TextEditor::TypeSetter::isVisPosOverGlyph(VisPos pos) const {
	return pos.row < totalRows && pos.column <= at(pos.row).columns;
}


//
//	TextEditor::AutoComplete::setConfig
//

void TextEditor::AutoComplete::setConfig(const AutoCompleteConfig* config) {
	if (config) {
		configuration = *config;

		// decode extra trigger and word characters
		auto decode = [](const std::string_view& text, std::vector<ImWchar>& codepoints) {
			codepoints.clear();

			for (auto i = text.begin(); i < text.end();) {
				ImWchar codepoint;
				i = CodePoint::read(i, text.end(), &codepoint);
				codepoints.emplace_back(codepoint);
			}
		};

		decode(configuration.triggerCharacters, triggerCodepoints);
		decode(configuration.wordCharacters, wordCodepoints);
		configured = true;

	} else {
		configured = false;
	}

	active = false;
}


//
//	TextEditor::AutoComplete::startTyping
//

bool TextEditor::AutoComplete::startTyping(Cursors& cursors, ImWchar character) {
	const auto trigger = isTriggerCharacter(character);

	// a pending activation follows typing that continues the search term
	if (requestActivation) {
		if (trigger || isWordCharacter(character)) {
			currentLocation = cursors.getMain().getSelectionEnd();
		}

		return false;
	}

	// a trigger character that isn't part of the search term ends the active session, so start a new one
	const auto restart = active && trigger && !isWordCharacter(character);

	if ((!active || restart) && (trigger || CodePoint::isWord(character)) && configured && configuration.triggerOnTyping) {
		triggeredManually = false;
		cancel();
		start(cursors);
		return true;

	} else {
		return false;
	}
}


//
//	TextEditor::AutoComplete::startShortcut
//

bool TextEditor::AutoComplete::startShortcut(Cursors& cursors) {
	if (!active && !requestActivation && configured && configuration.triggerOnShortcut) {
		triggeredManually = true;
		start(cursors);
		return true;

	} else {
		return false;
	}
}


//
//	TextEditor::AutoComplete::cancel
//

void TextEditor::AutoComplete::cancel() {
	if (active) {
		requestDeactivation = true;
	}
}


//
//	renderSuggestion
//

static bool renderSuggestion(const std::string_view& suggestion, const std::string_view& searchTerm, float width, bool selected) {
	// custom widget to render an autocomplete suggestion in the style of Visual Studio Code
	auto glyphPos = ImGui::GetCursorScreenPos();
	const auto size = ImVec2(width, ImGui::GetFrameHeightWithSpacing());
	const auto clicked = ImGui::InvisibleButton("suggestion", size);

	auto drawList = ImGui::GetWindowDrawList();
	const auto font = ImGui::GetFont();
	const auto fontSize = ImGui::GetFontSize();
	const auto glyphWidth = ImGui::CalcTextSize("#").x;

	// highlight selected item
	if (selected) {
		drawList->AddRectFilled(glyphPos, glyphPos + size, ImGui::GetColorU32(ImGuiCol_Header));
	}

	// process all UTF-8 glyphs in suggestion
	glyphPos += ImGui::GetStyle().FramePadding;
	const auto suggestionEnd = suggestion.end();
	const auto searchTermEnd = searchTerm.end();
	auto i = TextEditor::CodePoint::skipBOM(suggestion.begin(), suggestionEnd);
	auto j = TextEditor::CodePoint::skipBOM(searchTerm.begin(), searchTermEnd);

	while (i < suggestionEnd) {
		// get next glyph from suggestion
		ImWchar codepoint;
		i = TextEditor::CodePoint::read(i, suggestionEnd, &codepoint);

		// highlight glyph in suggestion that match search term
		auto color = ImGui::GetColorU32(ImGuiCol_Text);

		if (j < searchTermEnd) {
			ImWchar searchCodePoint;
			const auto next = TextEditor::CodePoint::read(j, searchTermEnd, &searchCodePoint);

			if (TextEditor::CodePoint::toLower(searchCodePoint) == TextEditor::CodePoint::toLower(codepoint)) {
				color = ImGui::GetColorU32(ImGuiCol_TextLink);
				j = next;
			}
		}

		// render the glyph
		font->RenderChar(drawList, fontSize, glyphPos, color, codepoint);
		glyphPos.x += glyphWidth;
	}

	return clicked;
}


//
//	TextEditor::AutoComplete::render
//

bool TextEditor::AutoComplete::render(Document& document, Cursors& cursors, TypeSetter& typesetter, const Language* language, float textOffset, ImVec2 glyphSize) {
	// see if we need to activate autocomplete mode (after a previous session is closed)
	if (requestActivation && !active) {
		// drop the request if the editor lost focus (e.g. a menu was opened)
		// or the cursor was moved by anything but typing a search term during the delay
		if (!ImGui::IsWindowFocused() || cursors.getMain().getSelectionEnd() != currentLocation) {
			requestActivation = false;

		// apply popup delay
		} else if (std::chrono::system_clock::now() > activationTime) {
			// reset activation flag
			requestActivation = false;

			// capture locations
			startLocation = findWordStart(document, currentLocation);

			// update the autocomplete state
			updateState(document, language);

			// handle cases where autocomplete request is ignored
			if(state.inComment && !configuration.triggerInComments) {
				return false;
			}

			if(state.inString && !configuration.triggerInStrings) {
				return false;
			}

			// get initial list of suggestions from the app
			refreshSuggestions();

			// show autocomplete popup window
			ImGui::OpenPopup("AutoCompleteContextMenu");
			active = true;
		}
	}

	// only continue if autocomplete is active
	if (!active) {
		return false;
	}

	// see if cursor moved since last time
	const auto newLocation = cursors.getMain().getSelectionEnd();

	if (newLocation != currentLocation) {
		// see if we need to deactivate autocomplete because cursor is on new line
		if (newLocation.line != currentLocation.line) {
			requestDeactivation = true;

		} else {
			// see if cursor moved away from current word
			const auto newStart = findWordStart(document, newLocation);

			if (newStart == startLocation) {
				currentLocation = newLocation;

				// we deactivate autocomplete if the current location is the start
				if (currentLocation == startLocation) {
					requestDeactivation = true;

				} else {
					updateState(document, language);
					refreshSuggestions();
				}

			} else {
				requestDeactivation = true;
			}
		}
	}

	// open popup window
	bool result = false;
	const auto pos = typesetter.docPos2VisPos(document, currentLocation);

	ImGui::SetNextWindowPos(ImVec2(
		ImGui::GetCursorScreenPos().x + textOffset + pos.column * glyphSize.x,
		ImGui::GetCursorScreenPos().y + (pos.row + 1) * glyphSize.y));

	const auto suggestions = state.suggestions.size();

	// an empty result while typing dismisses silently; only a manual trigger earns the "no suggestions" feedback
	if (suggestions == 0 && !state.suggestionsPromise && !triggeredManually) {
		requestDeactivation = true;
	}

	const auto visibleSuggestions = (suggestions == 0) ? 1 : std::min(static_cast<size_t>(10), suggestions);
	const auto& style = ImGui::GetStyle();
	const auto height = ImGui::GetFrameHeightWithSpacing() * visibleSuggestions + style.WindowPadding.y * 2.0f;
	ImGui::SetNextWindowSize(ImVec2(configuration.suggestionWidth * glyphSize.x, height));

	const ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav;

	if (ImGui::BeginPopup("AutoCompleteContextMenu", flags)) {
		if (ImGui::IsWindowAppearing()) {
			ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
		}

		// deactivate popup (if requested)
		if (requestDeactivation) {
			ImGui::CloseCurrentPopup();
			requestDeactivation = false;
			active = false;

		} else {
			// do we have any suggestions
			if (suggestions) {
				const auto items = state.suggestions.size();
				auto scroll = false;

				// apply arrow keys to selected suggestion
				if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
					scroll = true;

					if (currentSelection == 0) {
						currentSelection = items - 1;

					} else {
						currentSelection--;
					}

				} else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
					scroll = true;

					if (currentSelection == items - 1) {
						currentSelection = 0;

					} else {
						currentSelection++;
					}

				// use selected suggestion if user hit tab of return
				} else if (ImGui::IsKeyPressed(ImGuiKey_Tab) || ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
					requestDeactivation = true;
					result = true;

				} else if (configuration.autoInsertSingleSuggestions && triggeredManually && state.suggestions.size() == 1) {
					requestDeactivation = true;
					result = true;
				}

				// render suggestions (or their labels)
				const auto& texts = state.labels.size() == items ? state.labels : state.suggestions;

				// highlight the text that will be replaced (the callback could have moved its start)
				const auto highlightTerm = document.getSectionText(getReplacementStart(), currentLocation);

				for (size_t i = 0; i < items; i++) {
					// ensure unique ID
					ImGui::PushID(static_cast<int>(i));

					// scroll list to selected item (if required)
					const auto selected = i == currentSelection;

					if (scroll && selected) {
						ImGui::SetScrollHereY(0.5f);
					}

					if (renderSuggestion(texts[i], highlightTerm, ImGui::GetContentRegionAvail().x, selected)) {
						// user clicked on a suggestion, use it
						currentSelection = i;
						requestDeactivation = true;
						result = true;
					}

					ImGui::PopID();
				}

			} else {
				ImGui::TextUnformatted(configuration.noSuggestionsLabel.c_str());
			}
		}

		ImGui::EndPopup();

	} else {
		requestDeactivation = false;
		active = false;
	}

	return result;
}


//
//	TextEditor::AutoComplete::setSuggestions
//

void TextEditor::AutoComplete::setSuggestions(const std::vector<std::string>& suggestions, const std::vector<std::string>& labels) {
	state.suggestions = suggestions;
	state.labels = labels;
	currentSelection = 0;
}


//
//	TextEditor::AutoComplete::isTriggerCharacter
//

bool TextEditor::AutoComplete::isTriggerCharacter(ImWchar codepoint) const {
	return std::find(triggerCodepoints.begin(), triggerCodepoints.end(), codepoint) != triggerCodepoints.end();
}


//
//	TextEditor::AutoComplete::getReplacementStart
//

TextEditor::DocPos TextEditor::AutoComplete::getReplacementStart() const {
	// use the replacement start (which could have been changed by the callback) if it is valid
	const auto start = state.replacementStart;

	if (start.line == currentLocation.line && start.index <= currentLocation.index) {
		return start;

	} else {
		return startLocation;
	}
}


//
//	TextEditor::AutoComplete::getReplacementEnd
//

TextEditor::DocPos TextEditor::AutoComplete::getReplacementEnd(const Document& document) const {
	// use the replacement end (which could have been changed by the callback) if it is valid
	const auto end = state.replacementEnd;

	if (end.line == currentLocation.line && end.index >= currentLocation.index && end.index <= document[end.line].size()) {
		return end;

	} else {
		return findWordEnd(document, currentLocation);
	}
}


//
//	TextEditor::AutoComplete::isSpecialKeyPressed
//

bool TextEditor::AutoComplete::isSpecialKeyPressed() {
	for (const auto key : {ImGuiKey_Tab, ImGuiKey_Enter, ImGuiKey_KeypadEnter, ImGuiKey_UpArrow, ImGuiKey_DownArrow}) {
		if (ImGui::IsKeyPressed(key)) {
			return true;
		}
	}

	return false;
}


//
//	TextEditor::AutoComplete::start
//

void TextEditor::AutoComplete::start(Cursors& cursors) {
	// request start of autocomplete mode (can't be done here as the Dear ImGui context might not be right)
	requestActivation = true;
	currentLocation = cursors.getMain().getSelectionEnd();
	activationTime = std::chrono::system_clock::now() + configuration.triggerDelay;
}


//
//	TextEditor::AutoComplete::updateState
//

void TextEditor::AutoComplete::updateState(Document& document, const Language* language) {
	state.searchTerm = document.getSectionText(startLocation, currentLocation);

	if (currentLocation.index == 0) {
		state.inIdentifier = false;
		state.inNumber = false;

		const auto lineState = document[currentLocation.line].state;
		state.inComment = lineStateInComment(lineState);
		state.inString = lineStateInString(lineState);

	} else {
		const auto color = document.getColor(document.getLeft(currentLocation));
		state.inIdentifier = color == Color::identifier || color == Color::knownIdentifier;
		state.inNumber = color == Color::number;
		state.inComment = color == Color::comment;
		state.inString = color == Color::string;
	}

	state.searchTermStart = startLocation;
	state.searchTermEnd = currentLocation;
	state.replacementStart = startLocation;
	state.replacementEnd = findWordEnd(document, currentLocation);

	state.language = language;
	state.userData = configuration.userData;
}


//
//	TextEditor::AutoComplete::refreshSuggestions
//

void TextEditor::AutoComplete::refreshSuggestions() {
	// labels only apply to the suggestions from the call that set them
	state.labels.clear();

	// populate suggestion list through callback (or clear it if there is none)
	if (configuration.callback) {
		configuration.callback(state);

	} else {
		state.suggestions.clear();
	}

	currentSelection = 0;
}


//
//	TextEditor::AutoComplete::isWordCharacter
//

bool TextEditor::AutoComplete::isWordCharacter(ImWchar codepoint) const {
	return CodePoint::isWord(codepoint) || std::find(wordCodepoints.begin(), wordCodepoints.end(), codepoint) != wordCodepoints.end();
}


//
//	TextEditor::AutoComplete::findWordStart
//

TextEditor::DocPos TextEditor::AutoComplete::findWordStart(const Document& document, DocPos from) const {
	// like Document::findWordStart but with the extra word characters
	const auto& line = document[from.line];
	auto index = from.index;

	while (index > 0 && isWordCharacter(line[index - 1].codepoint)) {
		index--;
	}

	return DocPos(from.line, index);
}


//
//	TextEditor::AutoComplete::findWordEnd
//

TextEditor::DocPos TextEditor::AutoComplete::findWordEnd(const Document& document, DocPos from) const {
	// like Document::findWordEnd but with the extra word characters
	const auto& line = document[from.line];
	auto index = from.index;

	while (index < line.size() && isWordCharacter(line[index].codepoint)) {
		index++;
	}

	return DocPos(from.line, index);
}


//
//	Range table types
//

template <typename T>
struct Range {
	T low;
	T high;
};

using Range16 = Range<ImWchar16>;
using Range32 = Range<ImWchar32>;

template <typename T>
struct StrideRange {
	T low;
	T high;
	T stride;
};

using StrideRange16 = StrideRange<ImWchar16>;
using StrideRange32 = StrideRange<ImWchar32>;

template <typename T>
struct CaseRange {
	T low;
	T high;
	int32_t toUpper;
	int32_t toLower;
};

using CaseRange16 = CaseRange<char16_t>;
using CaseRange32 = CaseRange<char32_t>;


//
//	letters16
//

static StrideRange16 letters16[] = {
	{0x0041, 0x005a, 0x0001}, {0x0061, 0x007a, 0x0001}, {0x00aa, 0x00b5, 0x000b}, {0x00ba, 0x00c0, 0x0006},
	{0x00c1, 0x00d6, 0x0001}, {0x00d8, 0x00f6, 0x0001}, {0x00f8, 0x02c1, 0x0001}, {0x02c6, 0x02d1, 0x0001},
	{0x02e0, 0x02e4, 0x0001}, {0x02ec, 0x02ee, 0x0002}, {0x0370, 0x0374, 0x0001}, {0x0376, 0x0377, 0x0001},
	{0x037a, 0x037d, 0x0001}, {0x037f, 0x0386, 0x0007}, {0x0388, 0x038a, 0x0001}, {0x038c, 0x038e, 0x0002},
	{0x038f, 0x03a1, 0x0001}, {0x03a3, 0x03f5, 0x0001}, {0x03f7, 0x0481, 0x0001}, {0x048a, 0x052f, 0x0001},
	{0x0531, 0x0556, 0x0001}, {0x0558, 0x0559, 0x0001}, {0x0560, 0x0588, 0x0001}, {0x058b, 0x058c, 0x0001},
	{0x05d0, 0x05ea, 0x0001}, {0x05ef, 0x05f2, 0x0001}, {0x0620, 0x064a, 0x0001}, {0x066e, 0x066f, 0x0001},
	{0x0671, 0x06d3, 0x0001}, {0x06d5, 0x06e5, 0x0010}, {0x06e6, 0x06ee, 0x0008}, {0x06ef, 0x06fa, 0x000b},
	{0x06fb, 0x06fc, 0x0001}, {0x06ff, 0x0710, 0x0011}, {0x0712, 0x072f, 0x0001}, {0x074d, 0x07a5, 0x0001},
	{0x07b1, 0x07ca, 0x0019}, {0x07cb, 0x07ea, 0x0001}, {0x07f4, 0x07f5, 0x0001}, {0x07fa, 0x0800, 0x0006},
	{0x0801, 0x0815, 0x0001}, {0x081a, 0x0824, 0x000a}, {0x0828, 0x0840, 0x0018}, {0x0841, 0x0858, 0x0001},
	{0x0860, 0x086a, 0x0001}, {0x0870, 0x0887, 0x0001}, {0x0889, 0x088f, 0x0001}, {0x08a0, 0x08c9, 0x0001},
	{0x0904, 0x0939, 0x0001}, {0x093d, 0x0950, 0x0013}, {0x0958, 0x0961, 0x0001}, {0x0971, 0x0980, 0x0001},
	{0x0985, 0x098c, 0x0001}, {0x098f, 0x0990, 0x0001}, {0x0993, 0x09a8, 0x0001}, {0x09aa, 0x09b0, 0x0001},
	{0x09b2, 0x09b6, 0x0004}, {0x09b7, 0x09b9, 0x0001}, {0x09bd, 0x09ce, 0x0011}, {0x09dc, 0x09dd, 0x0001},
	{0x09df, 0x09e1, 0x0001}, {0x09f0, 0x09f1, 0x0001}, {0x09fc, 0x0a05, 0x0009}, {0x0a06, 0x0a0a, 0x0001},
	{0x0a0f, 0x0a10, 0x0001}, {0x0a13, 0x0a28, 0x0001}, {0x0a2a, 0x0a30, 0x0001}, {0x0a32, 0x0a33, 0x0001},
	{0x0a35, 0x0a36, 0x0001}, {0x0a38, 0x0a39, 0x0001}, {0x0a59, 0x0a5c, 0x0001}, {0x0a5e, 0x0a72, 0x0014},
	{0x0a73, 0x0a74, 0x0001}, {0x0a85, 0x0a8d, 0x0001}, {0x0a8f, 0x0a91, 0x0001}, {0x0a93, 0x0aa8, 0x0001},
	{0x0aaa, 0x0ab0, 0x0001}, {0x0ab2, 0x0ab3, 0x0001}, {0x0ab5, 0x0ab9, 0x0001}, {0x0abd, 0x0ad0, 0x0013},
	{0x0ae0, 0x0ae1, 0x0001}, {0x0af9, 0x0b05, 0x000c}, {0x0b06, 0x0b0c, 0x0001}, {0x0b0f, 0x0b10, 0x0001},
	{0x0b13, 0x0b28, 0x0001}, {0x0b2a, 0x0b30, 0x0001}, {0x0b32, 0x0b33, 0x0001}, {0x0b35, 0x0b39, 0x0001},
	{0x0b3d, 0x0b5c, 0x001f}, {0x0b5d, 0x0b5f, 0x0002}, {0x0b60, 0x0b61, 0x0001}, {0x0b71, 0x0b83, 0x0012},
	{0x0b85, 0x0b8a, 0x0001}, {0x0b8e, 0x0b90, 0x0001}, {0x0b92, 0x0b95, 0x0001}, {0x0b99, 0x0b9a, 0x0001},
	{0x0b9c, 0x0b9e, 0x0002}, {0x0b9f, 0x0ba3, 0x0004}, {0x0ba4, 0x0ba8, 0x0004}, {0x0ba9, 0x0baa, 0x0001},
	{0x0bae, 0x0bb9, 0x0001}, {0x0bd0, 0x0c05, 0x0035}, {0x0c06, 0x0c0c, 0x0001}, {0x0c0e, 0x0c10, 0x0001},
	{0x0c12, 0x0c28, 0x0001}, {0x0c2a, 0x0c39, 0x0001}, {0x0c3d, 0x0c58, 0x001b}, {0x0c59, 0x0c5a, 0x0001},
	{0x0c5c, 0x0c5d, 0x0001}, {0x0c60, 0x0c61, 0x0001}, {0x0c80, 0x0c85, 0x0005}, {0x0c86, 0x0c8c, 0x0001},
	{0x0c8e, 0x0c90, 0x0001}, {0x0c92, 0x0ca8, 0x0001}, {0x0caa, 0x0cb3, 0x0001}, {0x0cb5, 0x0cb9, 0x0001},
	{0x0cbd, 0x0cdc, 0x001f}, {0x0cdd, 0x0cde, 0x0001}, {0x0ce0, 0x0ce1, 0x0001}, {0x0cf1, 0x0cf2, 0x0001},
	{0x0d04, 0x0d0c, 0x0001}, {0x0d0e, 0x0d10, 0x0001}, {0x0d12, 0x0d3a, 0x0001}, {0x0d3d, 0x0d4e, 0x0011},
	{0x0d54, 0x0d56, 0x0001}, {0x0d5f, 0x0d61, 0x0001}, {0x0d7a, 0x0d7f, 0x0001}, {0x0d85, 0x0d96, 0x0001},
	{0x0d9a, 0x0db1, 0x0001}, {0x0db3, 0x0dbb, 0x0001}, {0x0dbd, 0x0dc0, 0x0003}, {0x0dc1, 0x0dc6, 0x0001},
	{0x0e01, 0x0e30, 0x0001}, {0x0e32, 0x0e33, 0x0001}, {0x0e40, 0x0e46, 0x0001}, {0x0e81, 0x0e82, 0x0001},
	{0x0e84, 0x0e86, 0x0002}, {0x0e87, 0x0e8a, 0x0001}, {0x0e8c, 0x0ea3, 0x0001}, {0x0ea5, 0x0ea7, 0x0002},
	{0x0ea8, 0x0eb0, 0x0001}, {0x0eb2, 0x0eb3, 0x0001}, {0x0ebd, 0x0ec0, 0x0003}, {0x0ec1, 0x0ec4, 0x0001},
	{0x0ec6, 0x0edc, 0x0016}, {0x0edd, 0x0edf, 0x0001}, {0x0f00, 0x0f40, 0x0040}, {0x0f41, 0x0f47, 0x0001},
	{0x0f49, 0x0f6c, 0x0001}, {0x0f88, 0x0f8c, 0x0001}, {0x1000, 0x102a, 0x0001}, {0x103f, 0x1050, 0x0011},
	{0x1051, 0x1055, 0x0001}, {0x105a, 0x105d, 0x0001}, {0x1061, 0x1065, 0x0004}, {0x1066, 0x106e, 0x0008},
	{0x106f, 0x1070, 0x0001}, {0x1075, 0x1081, 0x0001}, {0x108e, 0x10a0, 0x0012}, {0x10a1, 0x10c5, 0x0001},
	{0x10c7, 0x10cd, 0x0006}, {0x10d0, 0x10fa, 0x0001}, {0x10fc, 0x1248, 0x0001}, {0x124a, 0x124d, 0x0001},
	{0x1250, 0x1256, 0x0001}, {0x1258, 0x125a, 0x0002}, {0x125b, 0x125d, 0x0001}, {0x1260, 0x1288, 0x0001},
	{0x128a, 0x128d, 0x0001}, {0x1290, 0x12b0, 0x0001}, {0x12b2, 0x12b5, 0x0001}, {0x12b8, 0x12be, 0x0001},
	{0x12c0, 0x12c2, 0x0002}, {0x12c3, 0x12c5, 0x0001}, {0x12c8, 0x12d6, 0x0001}, {0x12d8, 0x1310, 0x0001},
	{0x1312, 0x1315, 0x0001}, {0x1318, 0x135a, 0x0001}, {0x1380, 0x138f, 0x0001}, {0x13a0, 0x13f5, 0x0001},
	{0x13f8, 0x13fd, 0x0001}, {0x1401, 0x166c, 0x0001}, {0x166f, 0x167f, 0x0001}, {0x1681, 0x169a, 0x0001},
	{0x16a0, 0x16ea, 0x0001}, {0x16f1, 0x16f8, 0x0001}, {0x1700, 0x1711, 0x0001}, {0x171f, 0x1731, 0x0001},
	{0x1740, 0x1751, 0x0001}, {0x1760, 0x176c, 0x0001}, {0x176e, 0x1770, 0x0001}, {0x1780, 0x17b3, 0x0001},
	{0x17d7, 0x17dc, 0x0005}, {0x1820, 0x1878, 0x0001}, {0x1880, 0x1884, 0x0001}, {0x1887, 0x18a8, 0x0001},
	{0x18aa, 0x18b0, 0x0006}, {0x18b1, 0x18f5, 0x0001}, {0x1900, 0x191e, 0x0001}, {0x1950, 0x196d, 0x0001},
	{0x1970, 0x1974, 0x0001}, {0x1980, 0x19ab, 0x0001}, {0x19b0, 0x19c9, 0x0001}, {0x1a00, 0x1a16, 0x0001},
	{0x1a20, 0x1a54, 0x0001}, {0x1aa7, 0x1b05, 0x005e}, {0x1b06, 0x1b33, 0x0001}, {0x1b45, 0x1b4c, 0x0001},
	{0x1b83, 0x1ba0, 0x0001}, {0x1bae, 0x1baf, 0x0001}, {0x1bba, 0x1be5, 0x0001}, {0x1c00, 0x1c23, 0x0001},
	{0x1c4d, 0x1c4f, 0x0001}, {0x1c5a, 0x1c7d, 0x0001}, {0x1c80, 0x1c8a, 0x0001}, {0x1c90, 0x1cba, 0x0001},
	{0x1cbd, 0x1cbf, 0x0001}, {0x1ce9, 0x1cec, 0x0001}, {0x1cee, 0x1cf3, 0x0001}, {0x1cf5, 0x1cf6, 0x0001},
	{0x1cfa, 0x1d00, 0x0006}, {0x1d01, 0x1dbf, 0x0001}, {0x1e00, 0x1f15, 0x0001}, {0x1f18, 0x1f1d, 0x0001},
	{0x1f20, 0x1f45, 0x0001}, {0x1f48, 0x1f4d, 0x0001}, {0x1f50, 0x1f57, 0x0001}, {0x1f59, 0x1f5f, 0x0002},
	{0x1f60, 0x1f7d, 0x0001}, {0x1f80, 0x1fb4, 0x0001}, {0x1fb6, 0x1fbc, 0x0001}, {0x1fbe, 0x1fc2, 0x0004},
	{0x1fc3, 0x1fc4, 0x0001}, {0x1fc6, 0x1fcc, 0x0001}, {0x1fd0, 0x1fd3, 0x0001}, {0x1fd6, 0x1fdb, 0x0001},
	{0x1fe0, 0x1fec, 0x0001}, {0x1ff2, 0x1ff4, 0x0001}, {0x1ff6, 0x1ffc, 0x0001}, {0x2071, 0x207f, 0x000e},
	{0x208f, 0x209f, 0x0001}, {0x2102, 0x2107, 0x0005}, {0x210a, 0x2113, 0x0001}, {0x2115, 0x2119, 0x0004},
	{0x211a, 0x211d, 0x0001}, {0x2124, 0x212a, 0x0002}, {0x212b, 0x212d, 0x0001}, {0x212f, 0x2139, 0x0001},
	{0x213c, 0x213f, 0x0001}, {0x2145, 0x2149, 0x0001}, {0x214e, 0x2183, 0x0035}, {0x2184, 0x2c00, 0x0a7c},
	{0x2c01, 0x2ce4, 0x0001}, {0x2ceb, 0x2cee, 0x0001}, {0x2cf2, 0x2cf3, 0x0001}, {0x2d00, 0x2d25, 0x0001},
	{0x2d27, 0x2d2d, 0x0006}, {0x2d30, 0x2d67, 0x0001}, {0x2d6f, 0x2d80, 0x0011}, {0x2d81, 0x2d96, 0x0001},
	{0x2da0, 0x2da6, 0x0001}, {0x2da8, 0x2dae, 0x0001}, {0x2db0, 0x2db6, 0x0001}, {0x2db8, 0x2dbe, 0x0001},
	{0x2dc0, 0x2dc6, 0x0001}, {0x2dc8, 0x2dce, 0x0001}, {0x2dd0, 0x2dd6, 0x0001}, {0x2dd8, 0x2dde, 0x0001},
	{0x2e2f, 0x3005, 0x01d6}, {0x3006, 0x3031, 0x002b}, {0x3032, 0x3035, 0x0001}, {0x303b, 0x303c, 0x0001},
	{0x3041, 0x3096, 0x0001}, {0x309d, 0x309f, 0x0001}, {0x30a1, 0x30fa, 0x0001}, {0x30fc, 0x30ff, 0x0001},
	{0x3105, 0x312f, 0x0001}, {0x3131, 0x318e, 0x0001}, {0x31a0, 0x31bf, 0x0001}, {0x31f0, 0x31ff, 0x0001},
	{0x3400, 0x4dbf, 0x19bf}, {0x4e00, 0x9fff, 0x51ff}, {0xa000, 0xa48c, 0x0001}, {0xa4d0, 0xa4fd, 0x0001},
	{0xa500, 0xa60c, 0x0001}, {0xa610, 0xa61f, 0x0001}, {0xa62a, 0xa62b, 0x0001}, {0xa640, 0xa66e, 0x0001},
	{0xa67f, 0xa69d, 0x0001}, {0xa6a0, 0xa6e5, 0x0001}, {0xa717, 0xa71f, 0x0001}, {0xa722, 0xa788, 0x0001},
	{0xa78b, 0xa7dd, 0x0001}, {0xa7e2, 0xa7f1, 0x000f}, {0xa7f2, 0xa801, 0x0001}, {0xa803, 0xa805, 0x0001},
	{0xa807, 0xa80a, 0x0001}, {0xa80c, 0xa822, 0x0001}, {0xa840, 0xa873, 0x0001}, {0xa882, 0xa8b3, 0x0001},
	{0xa8f2, 0xa8f7, 0x0001}, {0xa8fb, 0xa8fd, 0x0002}, {0xa8fe, 0xa90a, 0x000c}, {0xa90b, 0xa925, 0x0001},
	{0xa930, 0xa946, 0x0001}, {0xa960, 0xa97c, 0x0001}, {0xa984, 0xa9b2, 0x0001}, {0xa9cf, 0xa9e0, 0x0011},
	{0xa9e1, 0xa9e4, 0x0001}, {0xa9e6, 0xa9ef, 0x0001}, {0xa9fa, 0xa9fe, 0x0001}, {0xaa00, 0xaa28, 0x0001},
	{0xaa40, 0xaa42, 0x0001}, {0xaa44, 0xaa4b, 0x0001}, {0xaa60, 0xaa76, 0x0001}, {0xaa7a, 0xaa7e, 0x0004},
	{0xaa7f, 0xaaaf, 0x0001}, {0xaab1, 0xaab5, 0x0004}, {0xaab6, 0xaab9, 0x0003}, {0xaaba, 0xaabd, 0x0001},
	{0xaac0, 0xaac2, 0x0002}, {0xaadb, 0xaadd, 0x0001}, {0xaae0, 0xaaea, 0x0001}, {0xaaf2, 0xaaf4, 0x0001},
	{0xab01, 0xab06, 0x0001}, {0xab09, 0xab0e, 0x0001}, {0xab11, 0xab16, 0x0001}, {0xab20, 0xab26, 0x0001},
	{0xab28, 0xab2e, 0x0001}, {0xab30, 0xab5a, 0x0001}, {0xab5c, 0xab69, 0x0001}, {0xab6c, 0xab6d, 0x0001},
	{0xab70, 0xabe2, 0x0001}, {0xac00, 0xd7a3, 0x2ba3}, {0xd7b0, 0xd7c6, 0x0001}, {0xd7cb, 0xd7fb, 0x0001},
	{0xf900, 0xfa6d, 0x0001}, {0xfa70, 0xfad9, 0x0001}, {0xfb00, 0xfb06, 0x0001}, {0xfb13, 0xfb17, 0x0001},
	{0xfb1d, 0xfb1f, 0x0002}, {0xfb20, 0xfb28, 0x0001}, {0xfb2a, 0xfb36, 0x0001}, {0xfb38, 0xfb3c, 0x0001},
	{0xfb3e, 0xfb40, 0x0002}, {0xfb41, 0xfb43, 0x0002}, {0xfb44, 0xfb46, 0x0002}, {0xfb47, 0xfbb1, 0x0001},
	{0xfbd3, 0xfd3d, 0x0001}, {0xfd50, 0xfd8f, 0x0001}, {0xfd92, 0xfdc7, 0x0001}, {0xfdf0, 0xfdfb, 0x0001},
	{0xfe70, 0xfe74, 0x0001}, {0xfe76, 0xfefc, 0x0001}, {0xff21, 0xff3a, 0x0001}, {0xff41, 0xff5a, 0x0001},
	{0xff66, 0xffbe, 0x0001}, {0xffc2, 0xffc7, 0x0001}, {0xffca, 0xffcf, 0x0001}, {0xffd2, 0xffd7, 0x0001},
	{0xffda, 0xffdc, 0x0001}
};


//
//	letters32
//

#if defined(IMGUI_USE_WCHAR32)

static StrideRange32 letters32[] = {
	{0x10000, 0x1000b, 0x0001}, {0x1000d, 0x10026, 0x0001}, {0x10028, 0x1003a, 0x0001}, {0x1003c, 0x1003d, 0x0001},
	{0x1003f, 0x1004d, 0x0001}, {0x10050, 0x1005d, 0x0001}, {0x10080, 0x100fa, 0x0001}, {0x10280, 0x1029c, 0x0001},
	{0x102a0, 0x102d0, 0x0001}, {0x10300, 0x1031f, 0x0001}, {0x1032d, 0x10340, 0x0001}, {0x10342, 0x10349, 0x0001},
	{0x10350, 0x10375, 0x0001}, {0x10380, 0x1039d, 0x0001}, {0x103a0, 0x103c3, 0x0001}, {0x103c8, 0x103cf, 0x0001},
	{0x10400, 0x1049d, 0x0001}, {0x104b0, 0x104d3, 0x0001}, {0x104d8, 0x104fb, 0x0001}, {0x10500, 0x10527, 0x0001},
	{0x10530, 0x10563, 0x0001}, {0x10570, 0x1057a, 0x0001}, {0x1057c, 0x1058a, 0x0001}, {0x1058c, 0x10592, 0x0001},
	{0x10594, 0x10595, 0x0001}, {0x10597, 0x105a1, 0x0001}, {0x105a3, 0x105b1, 0x0001}, {0x105b3, 0x105b9, 0x0001},
	{0x105bb, 0x105bc, 0x0001}, {0x105c0, 0x105f3, 0x0001}, {0x10600, 0x10736, 0x0001}, {0x10740, 0x10755, 0x0001},
	{0x10760, 0x10767, 0x0001}, {0x10780, 0x10785, 0x0001}, {0x10787, 0x107b0, 0x0001}, {0x107b2, 0x107bf, 0x0001},
	{0x10800, 0x10805, 0x0001}, {0x10808, 0x1080a, 0x0002}, {0x1080b, 0x10835, 0x0001}, {0x10837, 0x10838, 0x0001},
	{0x1083c, 0x1083f, 0x0003}, {0x10840, 0x10855, 0x0001}, {0x10860, 0x10876, 0x0001}, {0x10880, 0x1089e, 0x0001},
	{0x108e0, 0x108f2, 0x0001}, {0x108f4, 0x108f5, 0x0001}, {0x10900, 0x10915, 0x0001}, {0x10920, 0x10939, 0x0001},
	{0x10940, 0x10959, 0x0001}, {0x10980, 0x109b7, 0x0001}, {0x109be, 0x109bf, 0x0001}, {0x10a00, 0x10a10, 0x0010},
	{0x10a11, 0x10a13, 0x0001}, {0x10a15, 0x10a17, 0x0001}, {0x10a19, 0x10a35, 0x0001}, {0x10a60, 0x10a7c, 0x0001},
	{0x10a80, 0x10a9c, 0x0001}, {0x10ac0, 0x10ac7, 0x0001}, {0x10ac9, 0x10ae4, 0x0001}, {0x10b00, 0x10b35, 0x0001},
	{0x10b40, 0x10b55, 0x0001}, {0x10b60, 0x10b72, 0x0001}, {0x10b80, 0x10b91, 0x0001}, {0x10c00, 0x10c48, 0x0001},
	{0x10c80, 0x10cb2, 0x0001}, {0x10cc0, 0x10cf2, 0x0001}, {0x10d00, 0x10d23, 0x0001}, {0x10d4a, 0x10d65, 0x0001},
	{0x10d6f, 0x10d85, 0x0001}, {0x10e80, 0x10ea9, 0x0001}, {0x10eb0, 0x10eb1, 0x0001}, {0x10ec2, 0x10ec7, 0x0001},
	{0x10ed9, 0x10eee, 0x0001}, {0x10f00, 0x10f1c, 0x0001}, {0x10f27, 0x10f30, 0x0009}, {0x10f31, 0x10f45, 0x0001},
	{0x10f70, 0x10f81, 0x0001}, {0x10fb0, 0x10fc4, 0x0001}, {0x10fe0, 0x10ff6, 0x0001}, {0x11003, 0x11037, 0x0001},
	{0x11071, 0x11072, 0x0001}, {0x11075, 0x11083, 0x000e}, {0x11084, 0x110af, 0x0001}, {0x110d0, 0x110e8, 0x0001},
	{0x11103, 0x11126, 0x0001}, {0x11144, 0x11147, 0x0003}, {0x11150, 0x11172, 0x0001}, {0x11176, 0x11183, 0x000d},
	{0x11184, 0x111b2, 0x0001}, {0x111c1, 0x111c4, 0x0001}, {0x111da, 0x111dc, 0x0002}, {0x11200, 0x11211, 0x0001},
	{0x11213, 0x1122b, 0x0001}, {0x1123f, 0x11240, 0x0001}, {0x11280, 0x11286, 0x0001}, {0x11288, 0x1128a, 0x0002},
	{0x1128b, 0x1128d, 0x0001}, {0x1128f, 0x1129d, 0x0001}, {0x1129f, 0x112a8, 0x0001}, {0x112b0, 0x112de, 0x0001},
	{0x11305, 0x1130c, 0x0001}, {0x1130f, 0x11310, 0x0001}, {0x11313, 0x11328, 0x0001}, {0x1132a, 0x11330, 0x0001},
	{0x11332, 0x11333, 0x0001}, {0x11335, 0x11339, 0x0001}, {0x1133d, 0x11350, 0x0013}, {0x1135d, 0x11361, 0x0001},
	{0x11380, 0x11389, 0x0001}, {0x1138b, 0x1138e, 0x0003}, {0x11390, 0x113b5, 0x0001}, {0x113b7, 0x113d1, 0x001a},
	{0x113d3, 0x11400, 0x002d}, {0x11401, 0x11434, 0x0001}, {0x11447, 0x1144a, 0x0001}, {0x1145f, 0x11461, 0x0001},
	{0x11480, 0x114af, 0x0001}, {0x114c4, 0x114c5, 0x0001}, {0x114c7, 0x11580, 0x00b9}, {0x11581, 0x115ae, 0x0001},
	{0x115d8, 0x115db, 0x0001}, {0x11600, 0x1162f, 0x0001}, {0x11644, 0x11680, 0x003c}, {0x11681, 0x116aa, 0x0001},
	{0x116b8, 0x11700, 0x0048}, {0x11701, 0x1171a, 0x0001}, {0x11740, 0x11746, 0x0001}, {0x11800, 0x1182b, 0x0001},
	{0x118a0, 0x118df, 0x0001}, {0x118ff, 0x11906, 0x0001}, {0x11909, 0x1190c, 0x0003}, {0x1190d, 0x11913, 0x0001},
	{0x11915, 0x11916, 0x0001}, {0x11918, 0x1192f, 0x0001}, {0x1193f, 0x11941, 0x0002}, {0x119a0, 0x119a7, 0x0001},
	{0x119aa, 0x119d0, 0x0001}, {0x119e1, 0x119e3, 0x0002}, {0x11a00, 0x11a0b, 0x000b}, {0x11a0c, 0x11a32, 0x0001},
	{0x11a3a, 0x11a50, 0x0016}, {0x11a5c, 0x11a89, 0x0001}, {0x11a9d, 0x11ab0, 0x0013}, {0x11ab1, 0x11af8, 0x0001},
	{0x11b0a, 0x11bc0, 0x00b6}, {0x11bc1, 0x11be0, 0x0001}, {0x11c00, 0x11c08, 0x0001}, {0x11c0a, 0x11c2e, 0x0001},
	{0x11c40, 0x11c72, 0x0032}, {0x11c73, 0x11c8f, 0x0001}, {0x11d00, 0x11d06, 0x0001}, {0x11d08, 0x11d09, 0x0001},
	{0x11d0b, 0x11d30, 0x0001}, {0x11d46, 0x11d60, 0x001a}, {0x11d61, 0x11d65, 0x0001}, {0x11d67, 0x11d68, 0x0001},
	{0x11d6a, 0x11d89, 0x0001}, {0x11d98, 0x11db0, 0x0018}, {0x11db1, 0x11ddb, 0x0001}, {0x11df1, 0x11ee0, 0x00ef},
	{0x11ee1, 0x11ef2, 0x0001}, {0x11f02, 0x11f04, 0x0002}, {0x11f05, 0x11f10, 0x0001}, {0x11f12, 0x11f33, 0x0001},
	{0x11fb0, 0x12000, 0x0050}, {0x12001, 0x12399, 0x0001}, {0x12480, 0x12543, 0x0001}, {0x12f90, 0x12ff0, 0x0001},
	{0x13000, 0x1342f, 0x0001}, {0x13441, 0x13446, 0x0001}, {0x13460, 0x143fa, 0x0001}, {0x14400, 0x14646, 0x0001},
	{0x16100, 0x1611d, 0x0001}, {0x16800, 0x16a38, 0x0001}, {0x16a40, 0x16a5e, 0x0001}, {0x16a70, 0x16abe, 0x0001},
	{0x16ad0, 0x16aed, 0x0001}, {0x16b00, 0x16b2f, 0x0001}, {0x16b40, 0x16b43, 0x0001}, {0x16b63, 0x16b77, 0x0001},
	{0x16b7d, 0x16b8f, 0x0001}, {0x16d40, 0x16d6c, 0x0001}, {0x16e40, 0x16e7f, 0x0001}, {0x16ea0, 0x16eb8, 0x0001},
	{0x16ebb, 0x16ed3, 0x0001}, {0x16f00, 0x16f4a, 0x0001}, {0x16f50, 0x16f93, 0x0043}, {0x16f94, 0x16f9f, 0x0001},
	{0x16fe0, 0x16fe1, 0x0001}, {0x16fe3, 0x16ff2, 0x000f}, {0x16ff3, 0x17000, 0x000d}, {0x187ff, 0x18cda, 0x0001},
	{0x18cff, 0x18d00, 0x0001}, {0x18d20, 0x18d80, 0x0060}, {0x18d81, 0x18df2, 0x0001}, {0x18e00, 0x19191, 0x0391},
	{0x191a0, 0x191d2, 0x0001}, {0x1aff0, 0x1aff3, 0x0001}, {0x1aff5, 0x1affb, 0x0001}, {0x1affd, 0x1affe, 0x0001},
	{0x1b000, 0x1b128, 0x0001}, {0x1b132, 0x1b150, 0x001e}, {0x1b151, 0x1b152, 0x0001}, {0x1b155, 0x1b164, 0x000f},
	{0x1b165, 0x1b168, 0x0001}, {0x1b170, 0x1b2fb, 0x0001}, {0x1bc00, 0x1bc6a, 0x0001}, {0x1bc70, 0x1bc7c, 0x0001},
	{0x1bc80, 0x1bc88, 0x0001}, {0x1bc90, 0x1bc99, 0x0001}, {0x1d400, 0x1d454, 0x0001}, {0x1d456, 0x1d49c, 0x0001},
	{0x1d49e, 0x1d49f, 0x0001}, {0x1d4a2, 0x1d4a5, 0x0003}, {0x1d4a6, 0x1d4a9, 0x0003}, {0x1d4aa, 0x1d4ac, 0x0001},
	{0x1d4ae, 0x1d4b9, 0x0001}, {0x1d4bb, 0x1d4bd, 0x0002}, {0x1d4be, 0x1d4c3, 0x0001}, {0x1d4c5, 0x1d505, 0x0001},
	{0x1d507, 0x1d50a, 0x0001}, {0x1d50d, 0x1d514, 0x0001}, {0x1d516, 0x1d51c, 0x0001}, {0x1d51e, 0x1d539, 0x0001},
	{0x1d53b, 0x1d53e, 0x0001}, {0x1d540, 0x1d544, 0x0001}, {0x1d546, 0x1d54a, 0x0004}, {0x1d54b, 0x1d550, 0x0001},
	{0x1d552, 0x1d6a6, 0x0001}, {0x1d6a8, 0x1d6c0, 0x0001}, {0x1d6c2, 0x1d6da, 0x0001}, {0x1d6dc, 0x1d6fa, 0x0001},
	{0x1d6fc, 0x1d714, 0x0001}, {0x1d716, 0x1d734, 0x0001}, {0x1d736, 0x1d74e, 0x0001}, {0x1d750, 0x1d76e, 0x0001},
	{0x1d770, 0x1d788, 0x0001}, {0x1d78a, 0x1d7a8, 0x0001}, {0x1d7aa, 0x1d7c2, 0x0001}, {0x1d7c4, 0x1d7cb, 0x0001},
	{0x1df00, 0x1df81, 0x0001}, {0x1df90, 0x1df96, 0x0001}, {0x1dfcd, 0x1dfff, 0x0001}, {0x1e030, 0x1e06d, 0x0001},
	{0x1e100, 0x1e12c, 0x0001}, {0x1e137, 0x1e13d, 0x0001}, {0x1e14e, 0x1e290, 0x0142}, {0x1e291, 0x1e2ad, 0x0001},
	{0x1e2c0, 0x1e2eb, 0x0001}, {0x1e4d0, 0x1e4eb, 0x0001}, {0x1e5d0, 0x1e5ed, 0x0001}, {0x1e5f0, 0x1e6c0, 0x00d0},
	{0x1e6c1, 0x1e6de, 0x0001}, {0x1e6e0, 0x1e6e2, 0x0001}, {0x1e6e4, 0x1e6e5, 0x0001}, {0x1e6e7, 0x1e6ed, 0x0001},
	{0x1e6f0, 0x1e6f4, 0x0001}, {0x1e6fe, 0x1e6ff, 0x0001}, {0x1e7e0, 0x1e7e6, 0x0001}, {0x1e7e8, 0x1e7eb, 0x0001},
	{0x1e7ed, 0x1e7ee, 0x0001}, {0x1e7f0, 0x1e7fe, 0x0001}, {0x1e800, 0x1e8c4, 0x0001}, {0x1e900, 0x1e943, 0x0001},
	{0x1e94b, 0x1ee00, 0x04b5}, {0x1ee01, 0x1ee03, 0x0001}, {0x1ee05, 0x1ee1f, 0x0001}, {0x1ee21, 0x1ee22, 0x0001},
	{0x1ee24, 0x1ee27, 0x0003}, {0x1ee29, 0x1ee32, 0x0001}, {0x1ee34, 0x1ee37, 0x0001}, {0x1ee39, 0x1ee3b, 0x0002},
	{0x1ee42, 0x1ee47, 0x0005}, {0x1ee49, 0x1ee4d, 0x0002}, {0x1ee4e, 0x1ee4f, 0x0001}, {0x1ee51, 0x1ee52, 0x0001},
	{0x1ee54, 0x1ee57, 0x0003}, {0x1ee59, 0x1ee61, 0x0002}, {0x1ee62, 0x1ee64, 0x0002}, {0x1ee67, 0x1ee6a, 0x0001},
	{0x1ee6c, 0x1ee72, 0x0001}, {0x1ee74, 0x1ee77, 0x0001}, {0x1ee79, 0x1ee7c, 0x0001}, {0x1ee7e, 0x1ee80, 0x0002},
	{0x1ee81, 0x1ee89, 0x0001}, {0x1ee8b, 0x1ee9b, 0x0001}, {0x1eea1, 0x1eea3, 0x0001}, {0x1eea5, 0x1eea9, 0x0001},
	{0x1eeab, 0x1eebb, 0x0001}, {0x20000, 0x2a6df, 0xa6df}, {0x2a700, 0x2b73f, 0x103f}, {0x2b740, 0x2b81e, 0x00de},
	{0x2b820, 0x2cead, 0x168d}, {0x2ceb0, 0x2ebe0, 0x1d30}, {0x2ebf0, 0x2ee5d, 0x026d}, {0x2f800, 0x2fa1d, 0x0001},
	{0x30000, 0x3134a, 0x134a}, {0x31350, 0x323af, 0x105f}, {0x323b0, 0x33479, 0x10c9}, {0x3d000, 0x3fc3f, 0x2c3f}
};

#endif


//
//	lower16
//

static StrideRange16 lower16[] = {
	{0x0061, 0x007a, 0x0001}, {0x00b5, 0x00df, 0x002a}, {0x00e0, 0x00f6, 0x0001}, {0x00f8, 0x00ff, 0x0001},
	{0x0101, 0x0137, 0x0002}, {0x0138, 0x0148, 0x0002}, {0x0149, 0x0177, 0x0002}, {0x017a, 0x017e, 0x0002},
	{0x017f, 0x0180, 0x0001}, {0x0183, 0x0185, 0x0002}, {0x0188, 0x018c, 0x0004}, {0x018d, 0x0192, 0x0005},
	{0x0195, 0x0199, 0x0004}, {0x019a, 0x019b, 0x0001}, {0x019e, 0x01a1, 0x0003}, {0x01a3, 0x01a5, 0x0002},
	{0x01a8, 0x01aa, 0x0002}, {0x01ab, 0x01ad, 0x0002}, {0x01b0, 0x01b4, 0x0004}, {0x01b6, 0x01b9, 0x0003},
	{0x01ba, 0x01bd, 0x0003}, {0x01be, 0x01bf, 0x0001}, {0x01c6, 0x01cc, 0x0003}, {0x01ce, 0x01dc, 0x0002},
	{0x01dd, 0x01ef, 0x0002}, {0x01f0, 0x01f3, 0x0003}, {0x01f5, 0x01f9, 0x0004}, {0x01fb, 0x0233, 0x0002},
	{0x0234, 0x0239, 0x0001}, {0x023c, 0x023f, 0x0003}, {0x0240, 0x0242, 0x0002}, {0x0247, 0x024f, 0x0002},
	{0x0250, 0x0293, 0x0001}, {0x0296, 0x02af, 0x0001}, {0x0371, 0x0373, 0x0002}, {0x0377, 0x037b, 0x0004},
	{0x037c, 0x037d, 0x0001}, {0x0390, 0x03ac, 0x001c}, {0x03ad, 0x03ce, 0x0001}, {0x03d0, 0x03d1, 0x0001},
	{0x03d5, 0x03d7, 0x0001}, {0x03d9, 0x03ef, 0x0002}, {0x03f0, 0x03f3, 0x0001}, {0x03f5, 0x03fb, 0x0003},
	{0x03fc, 0x0430, 0x0034}, {0x0431, 0x045f, 0x0001}, {0x0461, 0x0481, 0x0002}, {0x048b, 0x04bf, 0x0002},
	{0x04c2, 0x04ce, 0x0002}, {0x04cf, 0x052f, 0x0002}, {0x0560, 0x0588, 0x0001}, {0x10d0, 0x10fa, 0x0001},
	{0x10fd, 0x10ff, 0x0001}, {0x13f8, 0x13fd, 0x0001}, {0x1c80, 0x1c88, 0x0001}, {0x1c8a, 0x1d00, 0x0076},
	{0x1d01, 0x1d2b, 0x0001}, {0x1d6b, 0x1d77, 0x0001}, {0x1d79, 0x1d9a, 0x0001}, {0x1e01, 0x1e95, 0x0002},
	{0x1e96, 0x1e9d, 0x0001}, {0x1e9f, 0x1eff, 0x0002}, {0x1f00, 0x1f07, 0x0001}, {0x1f10, 0x1f15, 0x0001},
	{0x1f20, 0x1f27, 0x0001}, {0x1f30, 0x1f37, 0x0001}, {0x1f40, 0x1f45, 0x0001}, {0x1f50, 0x1f57, 0x0001},
	{0x1f60, 0x1f67, 0x0001}, {0x1f70, 0x1f7d, 0x0001}, {0x1f80, 0x1f87, 0x0001}, {0x1f90, 0x1f97, 0x0001},
	{0x1fa0, 0x1fa7, 0x0001}, {0x1fb0, 0x1fb4, 0x0001}, {0x1fb6, 0x1fb7, 0x0001}, {0x1fbe, 0x1fc2, 0x0004},
	{0x1fc3, 0x1fc4, 0x0001}, {0x1fc6, 0x1fc7, 0x0001}, {0x1fd0, 0x1fd3, 0x0001}, {0x1fd6, 0x1fd7, 0x0001},
	{0x1fe0, 0x1fe7, 0x0001}, {0x1ff2, 0x1ff4, 0x0001}, {0x1ff6, 0x1ff7, 0x0001}, {0x210a, 0x210e, 0x0004},
	{0x210f, 0x2113, 0x0004}, {0x212f, 0x2139, 0x0005}, {0x213c, 0x213d, 0x0001}, {0x2146, 0x2149, 0x0001},
	{0x214e, 0x2184, 0x0036}, {0x2c30, 0x2c5f, 0x0001}, {0x2c61, 0x2c65, 0x0004}, {0x2c66, 0x2c6c, 0x0002},
	{0x2c71, 0x2c73, 0x0002}, {0x2c74, 0x2c76, 0x0002}, {0x2c77, 0x2c7b, 0x0001}, {0x2c81, 0x2ce3, 0x0002},
	{0x2ce4, 0x2cec, 0x0008}, {0x2cee, 0x2cf3, 0x0005}, {0x2d00, 0x2d25, 0x0001}, {0x2d27, 0x2d2d, 0x0006},
	{0xa641, 0xa66d, 0x0002}, {0xa681, 0xa69b, 0x0002}, {0xa723, 0xa72f, 0x0002}, {0xa730, 0xa731, 0x0001},
	{0xa733, 0xa771, 0x0002}, {0xa772, 0xa778, 0x0001}, {0xa77a, 0xa77c, 0x0002}, {0xa77f, 0xa787, 0x0002},
	{0xa78c, 0xa78e, 0x0002}, {0xa791, 0xa793, 0x0002}, {0xa794, 0xa795, 0x0001}, {0xa797, 0xa7a9, 0x0002},
	{0xa7af, 0xa7b5, 0x0006}, {0xa7b7, 0xa7c3, 0x0002}, {0xa7c8, 0xa7ca, 0x0002}, {0xa7cd, 0xa7db, 0x0002},
	{0xa7f6, 0xa7fa, 0x0004}, {0xab30, 0xab5a, 0x0001}, {0xab60, 0xab68, 0x0001}, {0xab70, 0xabbf, 0x0001},
	{0xfb00, 0xfb06, 0x0001}, {0xfb13, 0xfb17, 0x0001}, {0xff41, 0xff5a, 0x0001}
};


//
//	lower32
//

#if defined(IMGUI_USE_WCHAR32)

static StrideRange32 lower32[] = {
	{0x10428, 0x1044f, 0x0001}, {0x104d8, 0x104fb, 0x0001}, {0x10597, 0x105a1, 0x0001}, {0x105a3, 0x105b1, 0x0001},
	{0x105b3, 0x105b9, 0x0001}, {0x105bb, 0x105bc, 0x0001}, {0x10cc0, 0x10cf2, 0x0001}, {0x10d70, 0x10d85, 0x0001},
	{0x118c0, 0x118df, 0x0001}, {0x16e60, 0x16e7f, 0x0001}, {0x16ebb, 0x16ed3, 0x0001}, {0x1d41a, 0x1d433, 0x0001},
	{0x1d44e, 0x1d454, 0x0001}, {0x1d456, 0x1d467, 0x0001}, {0x1d482, 0x1d49b, 0x0001}, {0x1d4b6, 0x1d4b9, 0x0001},
	{0x1d4bb, 0x1d4bd, 0x0002}, {0x1d4be, 0x1d4c3, 0x0001}, {0x1d4c5, 0x1d4cf, 0x0001}, {0x1d4ea, 0x1d503, 0x0001},
	{0x1d51e, 0x1d537, 0x0001}, {0x1d552, 0x1d56b, 0x0001}, {0x1d586, 0x1d59f, 0x0001}, {0x1d5ba, 0x1d5d3, 0x0001},
	{0x1d5ee, 0x1d607, 0x0001}, {0x1d622, 0x1d63b, 0x0001}, {0x1d656, 0x1d66f, 0x0001}, {0x1d68a, 0x1d6a6, 0x0001},
	{0x1d6c2, 0x1d6da, 0x0001}, {0x1d6dc, 0x1d6e1, 0x0001}, {0x1d6fc, 0x1d714, 0x0001}, {0x1d716, 0x1d71b, 0x0001},
	{0x1d736, 0x1d74e, 0x0001}, {0x1d750, 0x1d755, 0x0001}, {0x1d770, 0x1d788, 0x0001}, {0x1d78a, 0x1d78f, 0x0001},
	{0x1d7aa, 0x1d7c2, 0x0001}, {0x1d7c4, 0x1d7c9, 0x0001}, {0x1d7cb, 0x1df00, 0x0735}, {0x1df01, 0x1df09, 0x0001},
	{0x1df0b, 0x1df3f, 0x0001}, {0x1df41, 0x1df47, 0x0001}, {0x1df49, 0x1df4b, 0x0002}, {0x1df4c, 0x1df4e, 0x0002},
	{0x1df4f, 0x1df50, 0x0001}, {0x1df52, 0x1df67, 0x0001}, {0x1df69, 0x1df6f, 0x0002}, {0x1df70, 0x1df71, 0x0001},
	{0x1df73, 0x1df7f, 0x0002}, {0x1df90, 0x1df96, 0x0001}, {0x1e922, 0x1e943, 0x0001}
};

#endif


//
//	upper16
//

static StrideRange16 upper16[] = {
	{0x0041, 0x005a, 0x0001}, {0x00c0, 0x00d6, 0x0001}, {0x00d8, 0x00de, 0x0001}, {0x0100, 0x0136, 0x0002},
	{0x0139, 0x0147, 0x0002}, {0x014a, 0x0178, 0x0002}, {0x0179, 0x017d, 0x0002}, {0x0181, 0x0182, 0x0001},
	{0x0184, 0x0186, 0x0002}, {0x0187, 0x0189, 0x0002}, {0x018a, 0x018b, 0x0001}, {0x018e, 0x0191, 0x0001},
	{0x0193, 0x0194, 0x0001}, {0x0196, 0x0198, 0x0001}, {0x019c, 0x019d, 0x0001}, {0x019f, 0x01a0, 0x0001},
	{0x01a2, 0x01a6, 0x0002}, {0x01a7, 0x01a9, 0x0002}, {0x01ac, 0x01ae, 0x0002}, {0x01af, 0x01b1, 0x0002},
	{0x01b2, 0x01b3, 0x0001}, {0x01b5, 0x01b7, 0x0002}, {0x01b8, 0x01bc, 0x0004}, {0x01c4, 0x01cd, 0x0003},
	{0x01cf, 0x01db, 0x0002}, {0x01de, 0x01ee, 0x0002}, {0x01f1, 0x01f4, 0x0003}, {0x01f6, 0x01f8, 0x0001},
	{0x01fa, 0x0232, 0x0002}, {0x023a, 0x023b, 0x0001}, {0x023d, 0x023e, 0x0001}, {0x0241, 0x0243, 0x0002},
	{0x0244, 0x0246, 0x0001}, {0x0248, 0x024e, 0x0002}, {0x0370, 0x0372, 0x0002}, {0x0376, 0x037f, 0x0009},
	{0x0386, 0x0388, 0x0002}, {0x0389, 0x038a, 0x0001}, {0x038c, 0x038e, 0x0002}, {0x038f, 0x0391, 0x0002},
	{0x0392, 0x03a1, 0x0001}, {0x03a3, 0x03ab, 0x0001}, {0x03cf, 0x03d2, 0x0003}, {0x03d3, 0x03d4, 0x0001},
	{0x03d8, 0x03ee, 0x0002}, {0x03f4, 0x03f7, 0x0003}, {0x03f9, 0x03fa, 0x0001}, {0x03fd, 0x042f, 0x0001},
	{0x0460, 0x0480, 0x0002}, {0x048a, 0x04c0, 0x0002}, {0x04c1, 0x04cd, 0x0002}, {0x04d0, 0x052e, 0x0002},
	{0x0531, 0x0556, 0x0001}, {0x10a0, 0x10c5, 0x0001}, {0x10c7, 0x10cd, 0x0006}, {0x13a0, 0x13f5, 0x0001},
	{0x1c89, 0x1c90, 0x0007}, {0x1c91, 0x1cba, 0x0001}, {0x1cbd, 0x1cbf, 0x0001}, {0x1e00, 0x1e94, 0x0002},
	{0x1e9e, 0x1efe, 0x0002}, {0x1f08, 0x1f0f, 0x0001}, {0x1f18, 0x1f1d, 0x0001}, {0x1f28, 0x1f2f, 0x0001},
	{0x1f38, 0x1f3f, 0x0001}, {0x1f48, 0x1f4d, 0x0001}, {0x1f59, 0x1f5f, 0x0002}, {0x1f68, 0x1f6f, 0x0001},
	{0x1fb8, 0x1fbb, 0x0001}, {0x1fc8, 0x1fcb, 0x0001}, {0x1fd8, 0x1fdb, 0x0001}, {0x1fe8, 0x1fec, 0x0001},
	{0x1ff8, 0x1ffb, 0x0001}, {0x2102, 0x2107, 0x0005}, {0x210b, 0x210d, 0x0001}, {0x2110, 0x2112, 0x0001},
	{0x2115, 0x2119, 0x0004}, {0x211a, 0x211d, 0x0001}, {0x2124, 0x212a, 0x0002}, {0x212b, 0x212d, 0x0001},
	{0x2130, 0x2133, 0x0001}, {0x213e, 0x213f, 0x0001}, {0x2145, 0x2183, 0x003e}, {0x2c00, 0x2c2f, 0x0001},
	{0x2c60, 0x2c62, 0x0002}, {0x2c63, 0x2c64, 0x0001}, {0x2c67, 0x2c6d, 0x0002}, {0x2c6e, 0x2c70, 0x0001},
	{0x2c72, 0x2c75, 0x0003}, {0x2c7e, 0x2c80, 0x0001}, {0x2c82, 0x2ce2, 0x0002}, {0x2ceb, 0x2ced, 0x0002},
	{0x2cf2, 0xa640, 0x794e}, {0xa642, 0xa66c, 0x0002}, {0xa680, 0xa69a, 0x0002}, {0xa722, 0xa72e, 0x0002},
	{0xa732, 0xa76e, 0x0002}, {0xa779, 0xa77d, 0x0002}, {0xa77e, 0xa786, 0x0002}, {0xa78b, 0xa78d, 0x0002},
	{0xa790, 0xa792, 0x0002}, {0xa796, 0xa7aa, 0x0002}, {0xa7ab, 0xa7ae, 0x0001}, {0xa7b0, 0xa7b4, 0x0001},
	{0xa7b6, 0xa7c4, 0x0002}, {0xa7c5, 0xa7c7, 0x0001}, {0xa7c9, 0xa7cb, 0x0002}, {0xa7cc, 0xa7dc, 0x0002},
	{0xa7dd, 0xa7e2, 0x0005}, {0xa7f5, 0xab6c, 0x0377}, {0xab6d, 0xff21, 0x53b4}, {0xff22, 0xff3a, 0x0001}
};


//
//	upper32
//

#if defined(IMGUI_USE_WCHAR32)

static StrideRange32 upper32[] = {
	{0x10400, 0x10427, 0x0001}, {0x104b0, 0x104d3, 0x0001}, {0x10570, 0x1057a, 0x0001}, {0x1057c, 0x1058a, 0x0001},
	{0x1058c, 0x10592, 0x0001}, {0x10594, 0x10595, 0x0001}, {0x10c80, 0x10cb2, 0x0001}, {0x10d50, 0x10d65, 0x0001},
	{0x118a0, 0x118bf, 0x0001}, {0x16e40, 0x16e5f, 0x0001}, {0x16ea0, 0x16eb8, 0x0001}, {0x1d400, 0x1d419, 0x0001},
	{0x1d434, 0x1d44d, 0x0001}, {0x1d468, 0x1d481, 0x0001}, {0x1d49c, 0x1d49e, 0x0002}, {0x1d49f, 0x1d4a5, 0x0003},
	{0x1d4a6, 0x1d4a9, 0x0003}, {0x1d4aa, 0x1d4ac, 0x0001}, {0x1d4ae, 0x1d4b5, 0x0001}, {0x1d4d0, 0x1d4e9, 0x0001},
	{0x1d504, 0x1d505, 0x0001}, {0x1d507, 0x1d50a, 0x0001}, {0x1d50d, 0x1d514, 0x0001}, {0x1d516, 0x1d51c, 0x0001},
	{0x1d538, 0x1d539, 0x0001}, {0x1d53b, 0x1d53e, 0x0001}, {0x1d540, 0x1d544, 0x0001}, {0x1d546, 0x1d54a, 0x0004},
	{0x1d54b, 0x1d550, 0x0001}, {0x1d56c, 0x1d585, 0x0001}, {0x1d5a0, 0x1d5b9, 0x0001}, {0x1d5d4, 0x1d5ed, 0x0001},
	{0x1d608, 0x1d621, 0x0001}, {0x1d63c, 0x1d655, 0x0001}, {0x1d670, 0x1d689, 0x0001}, {0x1d6a8, 0x1d6c0, 0x0001},
	{0x1d6e2, 0x1d6fa, 0x0001}, {0x1d71c, 0x1d734, 0x0001}, {0x1d756, 0x1d76e, 0x0001}, {0x1d790, 0x1d7a8, 0x0001},
	{0x1d7ca, 0x1df40, 0x0776}, {0x1df48, 0x1df4a, 0x0002}, {0x1df4d, 0x1df51, 0x0004}, {0x1df68, 0x1df6e, 0x0002},
	{0x1df72, 0x1df7e, 0x0002}, {0x1e900, 0x1e921, 0x0001}
};

#endif


//
//	numbers16
//

static StrideRange16 numbers16[] = {
	{0x0030, 0x0039, 0x0001}, {0x0660, 0x0669, 0x0001}, {0x06f0, 0x06f9, 0x0001}, {0x07c0, 0x07c9, 0x0001},
	{0x0966, 0x096f, 0x0001}, {0x09e6, 0x09ef, 0x0001}, {0x0a66, 0x0a6f, 0x0001}, {0x0ae6, 0x0aef, 0x0001},
	{0x0b66, 0x0b6f, 0x0001}, {0x0be6, 0x0bef, 0x0001}, {0x0c66, 0x0c6f, 0x0001}, {0x0ce6, 0x0cef, 0x0001},
	{0x0d66, 0x0d6f, 0x0001}, {0x0de6, 0x0def, 0x0001}, {0x0e50, 0x0e59, 0x0001}, {0x0ed0, 0x0ed9, 0x0001},
	{0x0f20, 0x0f29, 0x0001}, {0x1040, 0x1049, 0x0001}, {0x1090, 0x1099, 0x0001}, {0x17e0, 0x17e9, 0x0001},
	{0x1810, 0x1819, 0x0001}, {0x1946, 0x194f, 0x0001}, {0x19d0, 0x19d9, 0x0001}, {0x1a80, 0x1a89, 0x0001},
	{0x1a90, 0x1a99, 0x0001}, {0x1b50, 0x1b59, 0x0001}, {0x1bb0, 0x1bb9, 0x0001}, {0x1c40, 0x1c49, 0x0001},
	{0x1c50, 0x1c59, 0x0001}, {0xa620, 0xa629, 0x0001}, {0xa8d0, 0xa8d9, 0x0001}, {0xa900, 0xa909, 0x0001},
	{0xa9d0, 0xa9d9, 0x0001}, {0xa9f0, 0xa9f9, 0x0001}, {0xaa50, 0xaa59, 0x0001}, {0xabf0, 0xabf9, 0x0001},
	{0xff10, 0xff19, 0x0001}
};


//
//	numbers32
//

#if defined(IMGUI_USE_WCHAR32)

static StrideRange32 numbers32[] = {
	{0x104a0, 0x104a9, 0x0001}, {0x10d30, 0x10d39, 0x0001}, {0x10d40, 0x10d49, 0x0001}, {0x11066, 0x1106f, 0x0001},
	{0x110f0, 0x110f9, 0x0001}, {0x11136, 0x1113f, 0x0001}, {0x111d0, 0x111d9, 0x0001}, {0x112f0, 0x112f9, 0x0001},
	{0x11450, 0x11459, 0x0001}, {0x114d0, 0x114d9, 0x0001}, {0x11650, 0x11659, 0x0001}, {0x116c0, 0x116c9, 0x0001},
	{0x116d0, 0x116e3, 0x0001}, {0x11730, 0x11739, 0x0001}, {0x118e0, 0x118e9, 0x0001}, {0x11950, 0x11959, 0x0001},
	{0x11bf0, 0x11bf9, 0x0001}, {0x11c50, 0x11c59, 0x0001}, {0x11d50, 0x11d59, 0x0001}, {0x11da0, 0x11da9, 0x0001},
	{0x11de0, 0x11de9, 0x0001}, {0x11f50, 0x11f59, 0x0001}, {0x16130, 0x16139, 0x0001}, {0x16a60, 0x16a69, 0x0001},
	{0x16ac0, 0x16ac9, 0x0001}, {0x16b50, 0x16b59, 0x0001}, {0x16d70, 0x16d79, 0x0001}, {0x1ccf0, 0x1ccf9, 0x0001},
	{0x1d7ce, 0x1d7ff, 0x0001}, {0x1e140, 0x1e149, 0x0001}, {0x1e2f0, 0x1e2f9, 0x0001}, {0x1e4f0, 0x1e4f9, 0x0001},
	{0x1e5f1, 0x1e5fa, 0x0001}, {0x1e950, 0x1e959, 0x0001}, {0x1fbf0, 0x1fbf9, 0x0001}
};

#endif


//
//	whitespace16
//

static StrideRange16 whitespace16[] = {
	{0x0009, 0x000d, 0x0001}, {0x0020, 0x0085, 0x0065}, {0x00a0, 0x1680, 0x15e0}, {0x2000, 0x200a, 0x0001},
	{0x2028, 0x2029, 0x0001}, {0x202f, 0x205f, 0x0030}, {0x3000, 0x3000, 0x0001}
};


//
//	xidStart16
//

static StrideRange16 xidStart16[] = {
	{0x0041, 0x005a, 0x0001}, {0x0061, 0x007a, 0x0001}, {0x00aa, 0x00b5, 0x000b}, {0x00ba, 0x00c0, 0x0006},
	{0x00c1, 0x00d6, 0x0001}, {0x00d8, 0x00f6, 0x0001}, {0x00f8, 0x02c1, 0x0001}, {0x02c6, 0x02d1, 0x0001},
	{0x02e0, 0x02e4, 0x0001}, {0x02ec, 0x02ee, 0x0002}, {0x0370, 0x0374, 0x0001}, {0x0376, 0x0377, 0x0001},
	{0x037b, 0x037d, 0x0001}, {0x037f, 0x0386, 0x0007}, {0x0388, 0x038a, 0x0001}, {0x038c, 0x038e, 0x0002},
	{0x038f, 0x03a1, 0x0001}, {0x03a3, 0x03f5, 0x0001}, {0x03f7, 0x0481, 0x0001}, {0x048a, 0x052f, 0x0001},
	{0x0531, 0x0556, 0x0001}, {0x0558, 0x0559, 0x0001}, {0x0560, 0x0588, 0x0001}, {0x058b, 0x058c, 0x0001},
	{0x05d0, 0x05ea, 0x0001}, {0x05ef, 0x05f2, 0x0001}, {0x0620, 0x064a, 0x0001}, {0x066e, 0x066f, 0x0001},
	{0x0671, 0x06d3, 0x0001}, {0x06d5, 0x06e5, 0x0010}, {0x06e6, 0x06ee, 0x0008}, {0x06ef, 0x06fa, 0x000b},
	{0x06fb, 0x06fc, 0x0001}, {0x06ff, 0x0710, 0x0011}, {0x0712, 0x072f, 0x0001}, {0x074d, 0x07a5, 0x0001},
	{0x07b1, 0x07ca, 0x0019}, {0x07cb, 0x07ea, 0x0001}, {0x07f4, 0x07f5, 0x0001}, {0x07fa, 0x0800, 0x0006},
	{0x0801, 0x0815, 0x0001}, {0x081a, 0x0824, 0x000a}, {0x0828, 0x0840, 0x0018}, {0x0841, 0x0858, 0x0001},
	{0x0860, 0x086a, 0x0001}, {0x0870, 0x0887, 0x0001}, {0x0889, 0x088f, 0x0001}, {0x08a0, 0x08c9, 0x0001},
	{0x0904, 0x0939, 0x0001}, {0x093d, 0x0950, 0x0013}, {0x0958, 0x0961, 0x0001}, {0x0971, 0x0980, 0x0001},
	{0x0985, 0x098c, 0x0001}, {0x098f, 0x0990, 0x0001}, {0x0993, 0x09a8, 0x0001}, {0x09aa, 0x09b0, 0x0001},
	{0x09b2, 0x09b6, 0x0004}, {0x09b7, 0x09b9, 0x0001}, {0x09bd, 0x09ce, 0x0011}, {0x09dc, 0x09dd, 0x0001},
	{0x09df, 0x09e1, 0x0001}, {0x09f0, 0x09f1, 0x0001}, {0x09fc, 0x0a05, 0x0009}, {0x0a06, 0x0a0a, 0x0001},
	{0x0a0f, 0x0a10, 0x0001}, {0x0a13, 0x0a28, 0x0001}, {0x0a2a, 0x0a30, 0x0001}, {0x0a32, 0x0a33, 0x0001},
	{0x0a35, 0x0a36, 0x0001}, {0x0a38, 0x0a39, 0x0001}, {0x0a59, 0x0a5c, 0x0001}, {0x0a5e, 0x0a72, 0x0014},
	{0x0a73, 0x0a74, 0x0001}, {0x0a85, 0x0a8d, 0x0001}, {0x0a8f, 0x0a91, 0x0001}, {0x0a93, 0x0aa8, 0x0001},
	{0x0aaa, 0x0ab0, 0x0001}, {0x0ab2, 0x0ab3, 0x0001}, {0x0ab5, 0x0ab9, 0x0001}, {0x0abd, 0x0ad0, 0x0013},
	{0x0ae0, 0x0ae1, 0x0001}, {0x0af9, 0x0b05, 0x000c}, {0x0b06, 0x0b0c, 0x0001}, {0x0b0f, 0x0b10, 0x0001},
	{0x0b13, 0x0b28, 0x0001}, {0x0b2a, 0x0b30, 0x0001}, {0x0b32, 0x0b33, 0x0001}, {0x0b35, 0x0b39, 0x0001},
	{0x0b3d, 0x0b5c, 0x001f}, {0x0b5d, 0x0b5f, 0x0002}, {0x0b60, 0x0b61, 0x0001}, {0x0b71, 0x0b83, 0x0012},
	{0x0b85, 0x0b8a, 0x0001}, {0x0b8e, 0x0b90, 0x0001}, {0x0b92, 0x0b95, 0x0001}, {0x0b99, 0x0b9a, 0x0001},
	{0x0b9c, 0x0b9e, 0x0002}, {0x0b9f, 0x0ba3, 0x0004}, {0x0ba4, 0x0ba8, 0x0004}, {0x0ba9, 0x0baa, 0x0001},
	{0x0bae, 0x0bb9, 0x0001}, {0x0bd0, 0x0c05, 0x0035}, {0x0c06, 0x0c0c, 0x0001}, {0x0c0e, 0x0c10, 0x0001},
	{0x0c12, 0x0c28, 0x0001}, {0x0c2a, 0x0c39, 0x0001}, {0x0c3d, 0x0c58, 0x001b}, {0x0c59, 0x0c5a, 0x0001},
	{0x0c5c, 0x0c5d, 0x0001}, {0x0c60, 0x0c61, 0x0001}, {0x0c80, 0x0c85, 0x0005}, {0x0c86, 0x0c8c, 0x0001},
	{0x0c8e, 0x0c90, 0x0001}, {0x0c92, 0x0ca8, 0x0001}, {0x0caa, 0x0cb3, 0x0001}, {0x0cb5, 0x0cb9, 0x0001},
	{0x0cbd, 0x0cdc, 0x001f}, {0x0cdd, 0x0cde, 0x0001}, {0x0ce0, 0x0ce1, 0x0001}, {0x0cf1, 0x0cf2, 0x0001},
	{0x0d04, 0x0d0c, 0x0001}, {0x0d0e, 0x0d10, 0x0001}, {0x0d12, 0x0d3a, 0x0001}, {0x0d3d, 0x0d4e, 0x0011},
	{0x0d54, 0x0d56, 0x0001}, {0x0d5f, 0x0d61, 0x0001}, {0x0d7a, 0x0d7f, 0x0001}, {0x0d85, 0x0d96, 0x0001},
	{0x0d9a, 0x0db1, 0x0001}, {0x0db3, 0x0dbb, 0x0001}, {0x0dbd, 0x0dc0, 0x0003}, {0x0dc1, 0x0dc6, 0x0001},
	{0x0e01, 0x0e30, 0x0001}, {0x0e32, 0x0e40, 0x000e}, {0x0e41, 0x0e46, 0x0001}, {0x0e81, 0x0e82, 0x0001},
	{0x0e84, 0x0e86, 0x0002}, {0x0e87, 0x0e8a, 0x0001}, {0x0e8c, 0x0ea3, 0x0001}, {0x0ea5, 0x0ea7, 0x0002},
	{0x0ea8, 0x0eb0, 0x0001}, {0x0eb2, 0x0ebd, 0x000b}, {0x0ec0, 0x0ec4, 0x0001}, {0x0ec6, 0x0edc, 0x0016},
	{0x0edd, 0x0edf, 0x0001}, {0x0f00, 0x0f40, 0x0040}, {0x0f41, 0x0f47, 0x0001}, {0x0f49, 0x0f6c, 0x0001},
	{0x0f88, 0x0f8c, 0x0001}, {0x1000, 0x102a, 0x0001}, {0x103f, 0x1050, 0x0011}, {0x1051, 0x1055, 0x0001},
	{0x105a, 0x105d, 0x0001}, {0x1061, 0x1065, 0x0004}, {0x1066, 0x106e, 0x0008}, {0x106f, 0x1070, 0x0001},
	{0x1075, 0x1081, 0x0001}, {0x108e, 0x10a0, 0x0012}, {0x10a1, 0x10c5, 0x0001}, {0x10c7, 0x10cd, 0x0006},
	{0x10d0, 0x10fa, 0x0001}, {0x10fc, 0x1248, 0x0001}, {0x124a, 0x124d, 0x0001}, {0x1250, 0x1256, 0x0001},
	{0x1258, 0x125a, 0x0002}, {0x125b, 0x125d, 0x0001}, {0x1260, 0x1288, 0x0001}, {0x128a, 0x128d, 0x0001},
	{0x1290, 0x12b0, 0x0001}, {0x12b2, 0x12b5, 0x0001}, {0x12b8, 0x12be, 0x0001}, {0x12c0, 0x12c2, 0x0002},
	{0x12c3, 0x12c5, 0x0001}, {0x12c8, 0x12d6, 0x0001}, {0x12d8, 0x1310, 0x0001}, {0x1312, 0x1315, 0x0001},
	{0x1318, 0x135a, 0x0001}, {0x1380, 0x138f, 0x0001}, {0x13a0, 0x13f5, 0x0001}, {0x13f8, 0x13fd, 0x0001},
	{0x1401, 0x166c, 0x0001}, {0x166f, 0x167f, 0x0001}, {0x1681, 0x169a, 0x0001}, {0x16a0, 0x16ea, 0x0001},
	{0x16ee, 0x16f8, 0x0001}, {0x1700, 0x1711, 0x0001}, {0x171f, 0x1731, 0x0001}, {0x1740, 0x1751, 0x0001},
	{0x1760, 0x176c, 0x0001}, {0x176e, 0x1770, 0x0001}, {0x1780, 0x17b3, 0x0001}, {0x17d7, 0x17dc, 0x0005},
	{0x1820, 0x1878, 0x0001}, {0x1880, 0x18a8, 0x0001}, {0x18aa, 0x18b0, 0x0006}, {0x18b1, 0x18f5, 0x0001},
	{0x1900, 0x191e, 0x0001}, {0x1950, 0x196d, 0x0001}, {0x1970, 0x1974, 0x0001}, {0x1980, 0x19ab, 0x0001},
	{0x19b0, 0x19c9, 0x0001}, {0x1a00, 0x1a16, 0x0001}, {0x1a20, 0x1a54, 0x0001}, {0x1aa7, 0x1b05, 0x005e},
	{0x1b06, 0x1b33, 0x0001}, {0x1b45, 0x1b4c, 0x0001}, {0x1b83, 0x1ba0, 0x0001}, {0x1bae, 0x1baf, 0x0001},
	{0x1bba, 0x1be5, 0x0001}, {0x1c00, 0x1c23, 0x0001}, {0x1c4d, 0x1c4f, 0x0001}, {0x1c5a, 0x1c7d, 0x0001},
	{0x1c80, 0x1c8a, 0x0001}, {0x1c90, 0x1cba, 0x0001}, {0x1cbd, 0x1cbf, 0x0001}, {0x1ce9, 0x1cec, 0x0001},
	{0x1cee, 0x1cf3, 0x0001}, {0x1cf5, 0x1cf6, 0x0001}, {0x1cfa, 0x1d00, 0x0006}, {0x1d01, 0x1dbf, 0x0001},
	{0x1e00, 0x1f15, 0x0001}, {0x1f18, 0x1f1d, 0x0001}, {0x1f20, 0x1f45, 0x0001}, {0x1f48, 0x1f4d, 0x0001},
	{0x1f50, 0x1f57, 0x0001}, {0x1f59, 0x1f5f, 0x0002}, {0x1f60, 0x1f7d, 0x0001}, {0x1f80, 0x1fb4, 0x0001},
	{0x1fb6, 0x1fbc, 0x0001}, {0x1fbe, 0x1fc2, 0x0004}, {0x1fc3, 0x1fc4, 0x0001}, {0x1fc6, 0x1fcc, 0x0001},
	{0x1fd0, 0x1fd3, 0x0001}, {0x1fd6, 0x1fdb, 0x0001}, {0x1fe0, 0x1fec, 0x0001}, {0x1ff2, 0x1ff4, 0x0001},
	{0x1ff6, 0x1ffc, 0x0001}, {0x2071, 0x207f, 0x000e}, {0x208f, 0x209f, 0x0001}, {0x2102, 0x2107, 0x0005},
	{0x210a, 0x2113, 0x0001}, {0x2115, 0x2118, 0x0003}, {0x2119, 0x211d, 0x0001}, {0x2124, 0x212a, 0x0002},
	{0x212b, 0x2139, 0x0001}, {0x213c, 0x213f, 0x0001}, {0x2145, 0x2149, 0x0001}, {0x214e, 0x2160, 0x0012},
	{0x2161, 0x2188, 0x0001}, {0x2c00, 0x2ce4, 0x0001}, {0x2ceb, 0x2cee, 0x0001}, {0x2cf2, 0x2cf3, 0x0001},
	{0x2d00, 0x2d25, 0x0001}, {0x2d27, 0x2d2d, 0x0006}, {0x2d30, 0x2d67, 0x0001}, {0x2d6f, 0x2d80, 0x0011},
	{0x2d81, 0x2d96, 0x0001}, {0x2da0, 0x2da6, 0x0001}, {0x2da8, 0x2dae, 0x0001}, {0x2db0, 0x2db6, 0x0001},
	{0x2db8, 0x2dbe, 0x0001}, {0x2dc0, 0x2dc6, 0x0001}, {0x2dc8, 0x2dce, 0x0001}, {0x2dd0, 0x2dd6, 0x0001},
	{0x2dd8, 0x2dde, 0x0001}, {0x3005, 0x3007, 0x0001}, {0x3021, 0x3029, 0x0001}, {0x3031, 0x3035, 0x0001},
	{0x3038, 0x303c, 0x0001}, {0x3041, 0x3096, 0x0001}, {0x309d, 0x309f, 0x0001}, {0x30a1, 0x30fa, 0x0001},
	{0x30fc, 0x30ff, 0x0001}, {0x3105, 0x312f, 0x0001}, {0x3131, 0x318e, 0x0001}, {0x31a0, 0x31bf, 0x0001},
	{0x31f0, 0x31ff, 0x0001}, {0x3400, 0x4dbf, 0x0001}, {0x4e00, 0xa48c, 0x0001}, {0xa4d0, 0xa4fd, 0x0001},
	{0xa500, 0xa60c, 0x0001}, {0xa610, 0xa61f, 0x0001}, {0xa62a, 0xa62b, 0x0001}, {0xa640, 0xa66e, 0x0001},
	{0xa67f, 0xa69d, 0x0001}, {0xa6a0, 0xa6ef, 0x0001}, {0xa717, 0xa71f, 0x0001}, {0xa722, 0xa788, 0x0001},
	{0xa78b, 0xa7dd, 0x0001}, {0xa7e2, 0xa7f1, 0x000f}, {0xa7f2, 0xa801, 0x0001}, {0xa803, 0xa805, 0x0001},
	{0xa807, 0xa80a, 0x0001}, {0xa80c, 0xa822, 0x0001}, {0xa840, 0xa873, 0x0001}, {0xa882, 0xa8b3, 0x0001},
	{0xa8f2, 0xa8f7, 0x0001}, {0xa8fb, 0xa8fd, 0x0002}, {0xa8fe, 0xa90a, 0x000c}, {0xa90b, 0xa925, 0x0001},
	{0xa930, 0xa946, 0x0001}, {0xa960, 0xa97c, 0x0001}, {0xa984, 0xa9b2, 0x0001}, {0xa9cf, 0xa9e0, 0x0011},
	{0xa9e1, 0xa9e4, 0x0001}, {0xa9e6, 0xa9ef, 0x0001}, {0xa9fa, 0xa9fe, 0x0001}, {0xaa00, 0xaa28, 0x0001},
	{0xaa40, 0xaa42, 0x0001}, {0xaa44, 0xaa4b, 0x0001}, {0xaa60, 0xaa76, 0x0001}, {0xaa7a, 0xaa7e, 0x0004},
	{0xaa7f, 0xaaaf, 0x0001}, {0xaab1, 0xaab5, 0x0004}, {0xaab6, 0xaab9, 0x0003}, {0xaaba, 0xaabd, 0x0001},
	{0xaac0, 0xaac2, 0x0002}, {0xaadb, 0xaadd, 0x0001}, {0xaae0, 0xaaea, 0x0001}, {0xaaf2, 0xaaf4, 0x0001},
	{0xab01, 0xab06, 0x0001}, {0xab09, 0xab0e, 0x0001}, {0xab11, 0xab16, 0x0001}, {0xab20, 0xab26, 0x0001},
	{0xab28, 0xab2e, 0x0001}, {0xab30, 0xab5a, 0x0001}, {0xab5c, 0xab69, 0x0001}, {0xab6c, 0xab6d, 0x0001},
	{0xab70, 0xabe2, 0x0001}, {0xac00, 0xd7a3, 0x0001}, {0xd7b0, 0xd7c6, 0x0001}, {0xd7cb, 0xd7fb, 0x0001},
	{0xf900, 0xfa6d, 0x0001}, {0xfa70, 0xfad9, 0x0001}, {0xfb00, 0xfb06, 0x0001}, {0xfb13, 0xfb17, 0x0001},
	{0xfb1d, 0xfb1f, 0x0002}, {0xfb20, 0xfb28, 0x0001}, {0xfb2a, 0xfb36, 0x0001}, {0xfb38, 0xfb3c, 0x0001},
	{0xfb3e, 0xfb40, 0x0002}, {0xfb41, 0xfb43, 0x0002}, {0xfb44, 0xfb46, 0x0002}, {0xfb47, 0xfbb1, 0x0001},
	{0xfbd3, 0xfc5d, 0x0001}, {0xfc64, 0xfd3d, 0x0001}, {0xfd50, 0xfd8f, 0x0001}, {0xfd92, 0xfdc7, 0x0001},
	{0xfdf0, 0xfdf9, 0x0001}, {0xfe71, 0xfe73, 0x0002}, {0xfe77, 0xfe7f, 0x0002}, {0xfe80, 0xfefc, 0x0001},
	{0xff21, 0xff3a, 0x0001}, {0xff41, 0xff5a, 0x0001}, {0xff66, 0xff9d, 0x0001}, {0xffa0, 0xffbe, 0x0001},
	{0xffc2, 0xffc7, 0x0001}, {0xffca, 0xffcf, 0x0001}, {0xffd2, 0xffd7, 0x0001}, {0xffda, 0xffdc, 0x0001}
};


//
//	xidStart32
//

#if defined(IMGUI_USE_WCHAR32)

static StrideRange32 xidStart32[] = {
	{0x10000, 0x1000b, 0x0001}, {0x1000d, 0x10026, 0x0001}, {0x10028, 0x1003a, 0x0001}, {0x1003c, 0x1003d, 0x0001},
	{0x1003f, 0x1004d, 0x0001}, {0x10050, 0x1005d, 0x0001}, {0x10080, 0x100fa, 0x0001}, {0x10140, 0x10174, 0x0001},
	{0x10280, 0x1029c, 0x0001}, {0x102a0, 0x102d0, 0x0001}, {0x10300, 0x1031f, 0x0001}, {0x1032d, 0x1034a, 0x0001},
	{0x10350, 0x10375, 0x0001}, {0x10380, 0x1039d, 0x0001}, {0x103a0, 0x103c3, 0x0001}, {0x103c8, 0x103cf, 0x0001},
	{0x103d1, 0x103d5, 0x0001}, {0x10400, 0x1049d, 0x0001}, {0x104b0, 0x104d3, 0x0001}, {0x104d8, 0x104fb, 0x0001},
	{0x10500, 0x10527, 0x0001}, {0x10530, 0x10563, 0x0001}, {0x10570, 0x1057a, 0x0001}, {0x1057c, 0x1058a, 0x0001},
	{0x1058c, 0x10592, 0x0001}, {0x10594, 0x10595, 0x0001}, {0x10597, 0x105a1, 0x0001}, {0x105a3, 0x105b1, 0x0001},
	{0x105b3, 0x105b9, 0x0001}, {0x105bb, 0x105bc, 0x0001}, {0x105c0, 0x105f3, 0x0001}, {0x10600, 0x10736, 0x0001},
	{0x10740, 0x10755, 0x0001}, {0x10760, 0x10767, 0x0001}, {0x10780, 0x10785, 0x0001}, {0x10787, 0x107b0, 0x0001},
	{0x107b2, 0x107bf, 0x0001}, {0x10800, 0x10805, 0x0001}, {0x10808, 0x1080a, 0x0002}, {0x1080b, 0x10835, 0x0001},
	{0x10837, 0x10838, 0x0001}, {0x1083c, 0x1083f, 0x0003}, {0x10840, 0x10855, 0x0001}, {0x10860, 0x10876, 0x0001},
	{0x10880, 0x1089e, 0x0001}, {0x108e0, 0x108f2, 0x0001}, {0x108f4, 0x108f5, 0x0001}, {0x10900, 0x10915, 0x0001},
	{0x10920, 0x10939, 0x0001}, {0x10940, 0x10959, 0x0001}, {0x10980, 0x109b7, 0x0001}, {0x109be, 0x109bf, 0x0001},
	{0x10a00, 0x10a10, 0x0010}, {0x10a11, 0x10a13, 0x0001}, {0x10a15, 0x10a17, 0x0001}, {0x10a19, 0x10a35, 0x0001},
	{0x10a60, 0x10a7c, 0x0001}, {0x10a80, 0x10a9c, 0x0001}, {0x10ac0, 0x10ac7, 0x0001}, {0x10ac9, 0x10ae4, 0x0001},
	{0x10b00, 0x10b35, 0x0001}, {0x10b40, 0x10b55, 0x0001}, {0x10b60, 0x10b72, 0x0001}, {0x10b80, 0x10b91, 0x0001},
	{0x10c00, 0x10c48, 0x0001}, {0x10c80, 0x10cb2, 0x0001}, {0x10cc0, 0x10cf2, 0x0001}, {0x10d00, 0x10d23, 0x0001},
	{0x10d4a, 0x10d65, 0x0001}, {0x10d6f, 0x10d85, 0x0001}, {0x10e80, 0x10ea9, 0x0001}, {0x10eb0, 0x10eb1, 0x0001},
	{0x10ec2, 0x10ec7, 0x0001}, {0x10ed9, 0x10eee, 0x0001}, {0x10f00, 0x10f1c, 0x0001}, {0x10f27, 0x10f30, 0x0009},
	{0x10f31, 0x10f45, 0x0001}, {0x10f70, 0x10f81, 0x0001}, {0x10fb0, 0x10fc4, 0x0001}, {0x10fe0, 0x10ff6, 0x0001},
	{0x11003, 0x11037, 0x0001}, {0x11071, 0x11072, 0x0001}, {0x11075, 0x11083, 0x000e}, {0x11084, 0x110af, 0x0001},
	{0x110d0, 0x110e8, 0x0001}, {0x11103, 0x11126, 0x0001}, {0x11144, 0x11147, 0x0003}, {0x11150, 0x11172, 0x0001},
	{0x11176, 0x11183, 0x000d}, {0x11184, 0x111b2, 0x0001}, {0x111c1, 0x111c4, 0x0001}, {0x111da, 0x111dc, 0x0002},
	{0x11200, 0x11211, 0x0001}, {0x11213, 0x1122b, 0x0001}, {0x1123f, 0x11240, 0x0001}, {0x11280, 0x11286, 0x0001},
	{0x11288, 0x1128a, 0x0002}, {0x1128b, 0x1128d, 0x0001}, {0x1128f, 0x1129d, 0x0001}, {0x1129f, 0x112a8, 0x0001},
	{0x112b0, 0x112de, 0x0001}, {0x11305, 0x1130c, 0x0001}, {0x1130f, 0x11310, 0x0001}, {0x11313, 0x11328, 0x0001},
	{0x1132a, 0x11330, 0x0001}, {0x11332, 0x11333, 0x0001}, {0x11335, 0x11339, 0x0001}, {0x1133d, 0x11350, 0x0013},
	{0x1135d, 0x11361, 0x0001}, {0x11380, 0x11389, 0x0001}, {0x1138b, 0x1138e, 0x0003}, {0x11390, 0x113b5, 0x0001},
	{0x113b7, 0x113d1, 0x001a}, {0x113d3, 0x11400, 0x002d}, {0x11401, 0x11434, 0x0001}, {0x11447, 0x1144a, 0x0001},
	{0x1145f, 0x11461, 0x0001}, {0x11480, 0x114af, 0x0001}, {0x114c4, 0x114c5, 0x0001}, {0x114c7, 0x11580, 0x00b9},
	{0x11581, 0x115ae, 0x0001}, {0x115d8, 0x115db, 0x0001}, {0x11600, 0x1162f, 0x0001}, {0x11644, 0x11680, 0x003c},
	{0x11681, 0x116aa, 0x0001}, {0x116b8, 0x11700, 0x0048}, {0x11701, 0x1171a, 0x0001}, {0x11740, 0x11746, 0x0001},
	{0x11800, 0x1182b, 0x0001}, {0x118a0, 0x118df, 0x0001}, {0x118ff, 0x11906, 0x0001}, {0x11909, 0x1190c, 0x0003},
	{0x1190d, 0x11913, 0x0001}, {0x11915, 0x11916, 0x0001}, {0x11918, 0x1192f, 0x0001}, {0x1193f, 0x11941, 0x0002},
	{0x119a0, 0x119a7, 0x0001}, {0x119aa, 0x119d0, 0x0001}, {0x119e1, 0x119e3, 0x0002}, {0x11a00, 0x11a0b, 0x000b},
	{0x11a0c, 0x11a32, 0x0001}, {0x11a3a, 0x11a50, 0x0016}, {0x11a5c, 0x11a89, 0x0001}, {0x11a9d, 0x11ab0, 0x0013},
	{0x11ab1, 0x11af8, 0x0001}, {0x11b0a, 0x11bc0, 0x00b6}, {0x11bc1, 0x11be0, 0x0001}, {0x11c00, 0x11c08, 0x0001},
	{0x11c0a, 0x11c2e, 0x0001}, {0x11c40, 0x11c72, 0x0032}, {0x11c73, 0x11c8f, 0x0001}, {0x11d00, 0x11d06, 0x0001},
	{0x11d08, 0x11d09, 0x0001}, {0x11d0b, 0x11d30, 0x0001}, {0x11d46, 0x11d60, 0x001a}, {0x11d61, 0x11d65, 0x0001},
	{0x11d67, 0x11d68, 0x0001}, {0x11d6a, 0x11d89, 0x0001}, {0x11d98, 0x11db0, 0x0018}, {0x11db1, 0x11ddb, 0x0001},
	{0x11df1, 0x11ee0, 0x00ef}, {0x11ee1, 0x11ef2, 0x0001}, {0x11f02, 0x11f04, 0x0002}, {0x11f05, 0x11f10, 0x0001},
	{0x11f12, 0x11f33, 0x0001}, {0x11fb0, 0x12000, 0x0050}, {0x12001, 0x12399, 0x0001}, {0x12400, 0x1246f, 0x0001},
	{0x12475, 0x12543, 0x0001}, {0x12550, 0x12686, 0x0001}, {0x12f90, 0x12ff0, 0x0001}, {0x13000, 0x1342f, 0x0001},
	{0x13441, 0x13446, 0x0001}, {0x13460, 0x143fa, 0x0001}, {0x14400, 0x14646, 0x0001}, {0x16100, 0x1611d, 0x0001},
	{0x16800, 0x16a38, 0x0001}, {0x16a40, 0x16a5e, 0x0001}, {0x16a70, 0x16abe, 0x0001}, {0x16ad0, 0x16aed, 0x0001},
	{0x16b00, 0x16b2f, 0x0001}, {0x16b40, 0x16b43, 0x0001}, {0x16b63, 0x16b77, 0x0001}, {0x16b7d, 0x16b8f, 0x0001},
	{0x16d40, 0x16d6c, 0x0001}, {0x16e40, 0x16e7f, 0x0001}, {0x16ea0, 0x16eb8, 0x0001}, {0x16ebb, 0x16ed3, 0x0001},
	{0x16f00, 0x16f4a, 0x0001}, {0x16f50, 0x16f93, 0x0043}, {0x16f94, 0x16f9f, 0x0001}, {0x16fe0, 0x16fe1, 0x0001},
	{0x16fe3, 0x16ff2, 0x000f}, {0x16ff3, 0x16ff6, 0x0001}, {0x17000, 0x18cda, 0x0001}, {0x18cff, 0x18d20, 0x0001},
	{0x18d80, 0x18df2, 0x0001}, {0x18e00, 0x19191, 0x0001}, {0x191a0, 0x191d2, 0x0001}, {0x1aff0, 0x1aff3, 0x0001},
	{0x1aff5, 0x1affb, 0x0001}, {0x1affd, 0x1affe, 0x0001}, {0x1b000, 0x1b128, 0x0001}, {0x1b132, 0x1b150, 0x001e},
	{0x1b151, 0x1b152, 0x0001}, {0x1b155, 0x1b164, 0x000f}, {0x1b165, 0x1b168, 0x0001}, {0x1b170, 0x1b2fb, 0x0001},
	{0x1bc00, 0x1bc6a, 0x0001}, {0x1bc70, 0x1bc7c, 0x0001}, {0x1bc80, 0x1bc88, 0x0001}, {0x1bc90, 0x1bc99, 0x0001},
	{0x1d400, 0x1d454, 0x0001}, {0x1d456, 0x1d49c, 0x0001}, {0x1d49e, 0x1d49f, 0x0001}, {0x1d4a2, 0x1d4a5, 0x0003},
	{0x1d4a6, 0x1d4a9, 0x0003}, {0x1d4aa, 0x1d4ac, 0x0001}, {0x1d4ae, 0x1d4b9, 0x0001}, {0x1d4bb, 0x1d4bd, 0x0002},
	{0x1d4be, 0x1d4c3, 0x0001}, {0x1d4c5, 0x1d505, 0x0001}, {0x1d507, 0x1d50a, 0x0001}, {0x1d50d, 0x1d514, 0x0001},
	{0x1d516, 0x1d51c, 0x0001}, {0x1d51e, 0x1d539, 0x0001}, {0x1d53b, 0x1d53e, 0x0001}, {0x1d540, 0x1d544, 0x0001},
	{0x1d546, 0x1d54a, 0x0004}, {0x1d54b, 0x1d550, 0x0001}, {0x1d552, 0x1d6a6, 0x0001}, {0x1d6a8, 0x1d6c0, 0x0001},
	{0x1d6c2, 0x1d6da, 0x0001}, {0x1d6dc, 0x1d6fa, 0x0001}, {0x1d6fc, 0x1d714, 0x0001}, {0x1d716, 0x1d734, 0x0001},
	{0x1d736, 0x1d74e, 0x0001}, {0x1d750, 0x1d76e, 0x0001}, {0x1d770, 0x1d788, 0x0001}, {0x1d78a, 0x1d7a8, 0x0001},
	{0x1d7aa, 0x1d7c2, 0x0001}, {0x1d7c4, 0x1d7cb, 0x0001}, {0x1df00, 0x1df81, 0x0001}, {0x1df90, 0x1df96, 0x0001},
	{0x1dfcd, 0x1dfff, 0x0001}, {0x1e030, 0x1e06d, 0x0001}, {0x1e100, 0x1e12c, 0x0001}, {0x1e137, 0x1e13d, 0x0001},
	{0x1e14e, 0x1e290, 0x0142}, {0x1e291, 0x1e2ad, 0x0001}, {0x1e2c0, 0x1e2eb, 0x0001}, {0x1e4d0, 0x1e4eb, 0x0001},
	{0x1e5d0, 0x1e5ed, 0x0001}, {0x1e5f0, 0x1e6c0, 0x00d0}, {0x1e6c1, 0x1e6de, 0x0001}, {0x1e6e0, 0x1e6e2, 0x0001},
	{0x1e6e4, 0x1e6e5, 0x0001}, {0x1e6e7, 0x1e6ed, 0x0001}, {0x1e6f0, 0x1e6f4, 0x0001}, {0x1e6fe, 0x1e6ff, 0x0001},
	{0x1e7e0, 0x1e7e6, 0x0001}, {0x1e7e8, 0x1e7eb, 0x0001}, {0x1e7ed, 0x1e7ee, 0x0001}, {0x1e7f0, 0x1e7fe, 0x0001},
	{0x1e800, 0x1e8c4, 0x0001}, {0x1e900, 0x1e943, 0x0001}, {0x1e94b, 0x1ee00, 0x04b5}, {0x1ee01, 0x1ee03, 0x0001},
	{0x1ee05, 0x1ee1f, 0x0001}, {0x1ee21, 0x1ee22, 0x0001}, {0x1ee24, 0x1ee27, 0x0003}, {0x1ee29, 0x1ee32, 0x0001},
	{0x1ee34, 0x1ee37, 0x0001}, {0x1ee39, 0x1ee3b, 0x0002}, {0x1ee42, 0x1ee47, 0x0005}, {0x1ee49, 0x1ee4d, 0x0002},
	{0x1ee4e, 0x1ee4f, 0x0001}, {0x1ee51, 0x1ee52, 0x0001}, {0x1ee54, 0x1ee57, 0x0003}, {0x1ee59, 0x1ee61, 0x0002},
	{0x1ee62, 0x1ee64, 0x0002}, {0x1ee67, 0x1ee6a, 0x0001}, {0x1ee6c, 0x1ee72, 0x0001}, {0x1ee74, 0x1ee77, 0x0001},
	{0x1ee79, 0x1ee7c, 0x0001}, {0x1ee7e, 0x1ee80, 0x0002}, {0x1ee81, 0x1ee89, 0x0001}, {0x1ee8b, 0x1ee9b, 0x0001},
	{0x1eea1, 0x1eea3, 0x0001}, {0x1eea5, 0x1eea9, 0x0001}, {0x1eeab, 0x1eebb, 0x0001}, {0x20000, 0x2a6df, 0x0001},
	{0x2a700, 0x2b81e, 0x0001}, {0x2b820, 0x2cead, 0x0001}, {0x2ceb0, 0x2ebe0, 0x0001}, {0x2ebf0, 0x2ee5d, 0x0001},
	{0x2f800, 0x2fa1d, 0x0001}, {0x30000, 0x3134a, 0x0001}, {0x31350, 0x33479, 0x0001}, {0x3d000, 0x3fc3f, 0x0001}
};

#endif


//
//	xidContinue16
//

static StrideRange16 xidContinue16[] = {
	{0x0030, 0x0039, 0x0001}, {0x0041, 0x005a, 0x0001}, {0x005f, 0x0061, 0x0002}, {0x0062, 0x007a, 0x0001},
	{0x00aa, 0x00b5, 0x000b}, {0x00b7, 0x00ba, 0x0003}, {0x00c0, 0x00d6, 0x0001}, {0x00d8, 0x00f6, 0x0001},
	{0x00f8, 0x02c1, 0x0001}, {0x02c6, 0x02d1, 0x0001}, {0x02e0, 0x02e4, 0x0001}, {0x02ec, 0x02ee, 0x0002},
	{0x0300, 0x0374, 0x0001}, {0x0376, 0x0377, 0x0001}, {0x037b, 0x037d, 0x0001}, {0x037f, 0x0386, 0x0007},
	{0x0387, 0x038a, 0x0001}, {0x038c, 0x038e, 0x0002}, {0x038f, 0x03a1, 0x0001}, {0x03a3, 0x03f5, 0x0001},
	{0x03f7, 0x0481, 0x0001}, {0x0483, 0x0487, 0x0001}, {0x048a, 0x052f, 0x0001}, {0x0531, 0x0556, 0x0001},
	{0x0558, 0x0559, 0x0001}, {0x0560, 0x0588, 0x0001}, {0x058b, 0x058c, 0x0001}, {0x0591, 0x05bd, 0x0001},
	{0x05bf, 0x05c1, 0x0002}, {0x05c2, 0x05c4, 0x0002}, {0x05c5, 0x05c7, 0x0002}, {0x05c8, 0x05c9, 0x0001},
	{0x05d0, 0x05ea, 0x0001}, {0x05ef, 0x05f2, 0x0001}, {0x0610, 0x061a, 0x0001}, {0x0620, 0x0669, 0x0001},
	{0x066e, 0x06d3, 0x0001}, {0x06d5, 0x06dc, 0x0001}, {0x06df, 0x06e8, 0x0001}, {0x06ea, 0x06fc, 0x0001},
	{0x06ff, 0x0710, 0x0011}, {0x0711, 0x074a, 0x0001}, {0x074d, 0x07b1, 0x0001}, {0x07c0, 0x07f5, 0x0001},
	{0x07fa, 0x0800, 0x0003}, {0x0801, 0x082d, 0x0001}, {0x0840, 0x085b, 0x0001}, {0x0860, 0x086a, 0x0001},
	{0x0870, 0x0887, 0x0001}, {0x0889, 0x088f, 0x0001}, {0x0897, 0x08e1, 0x0001}, {0x08e3, 0x0963, 0x0001},
	{0x0966, 0x096f, 0x0001}, {0x0971, 0x0983, 0x0001}, {0x0985, 0x098c, 0x0001}, {0x098f, 0x0990, 0x0001},
	{0x0993, 0x09a8, 0x0001}, {0x09aa, 0x09b0, 0x0001}, {0x09b2, 0x09b6, 0x0004}, {0x09b7, 0x09b9, 0x0001},
	{0x09bc, 0x09c4, 0x0001}, {0x09c7, 0x09c8, 0x0001}, {0x09cb, 0x09ce, 0x0001}, {0x09d7, 0x09dc, 0x0005},
	{0x09dd, 0x09df, 0x0002}, {0x09e0, 0x09e3, 0x0001}, {0x09e6, 0x09f1, 0x0001}, {0x09fc, 0x09fe, 0x0002},
	{0x0a01, 0x0a03, 0x0001}, {0x0a05, 0x0a0a, 0x0001}, {0x0a0f, 0x0a10, 0x0001}, {0x0a13, 0x0a28, 0x0001},
	{0x0a2a, 0x0a30, 0x0001}, {0x0a32, 0x0a33, 0x0001}, {0x0a35, 0x0a36, 0x0001}, {0x0a38, 0x0a39, 0x0001},
	{0x0a3c, 0x0a3e, 0x0002}, {0x0a3f, 0x0a42, 0x0001}, {0x0a47, 0x0a48, 0x0001}, {0x0a4b, 0x0a4d, 0x0001},
	{0x0a51, 0x0a59, 0x0008}, {0x0a5a, 0x0a5c, 0x0001}, {0x0a5e, 0x0a66, 0x0008}, {0x0a67, 0x0a75, 0x0001},
	{0x0a81, 0x0a83, 0x0001}, {0x0a85, 0x0a8d, 0x0001}, {0x0a8f, 0x0a91, 0x0001}, {0x0a93, 0x0aa8, 0x0001},
	{0x0aaa, 0x0ab0, 0x0001}, {0x0ab2, 0x0ab3, 0x0001}, {0x0ab5, 0x0ab9, 0x0001}, {0x0abc, 0x0ac5, 0x0001},
	{0x0ac7, 0x0ac9, 0x0001}, {0x0acb, 0x0acd, 0x0001}, {0x0ad0, 0x0ae0, 0x0010}, {0x0ae1, 0x0ae3, 0x0001},
	{0x0ae6, 0x0aef, 0x0001}, {0x0af9, 0x0aff, 0x0001}, {0x0b01, 0x0b03, 0x0001}, {0x0b05, 0x0b0c, 0x0001},
	{0x0b0f, 0x0b10, 0x0001}, {0x0b13, 0x0b28, 0x0001}, {0x0b2a, 0x0b30, 0x0001}, {0x0b32, 0x0b33, 0x0001},
	{0x0b35, 0x0b39, 0x0001}, {0x0b3c, 0x0b44, 0x0001}, {0x0b47, 0x0b48, 0x0001}, {0x0b4b, 0x0b4d, 0x0001},
	{0x0b53, 0x0b57, 0x0001}, {0x0b5c, 0x0b5d, 0x0001}, {0x0b5f, 0x0b63, 0x0001}, {0x0b66, 0x0b6f, 0x0001},
	{0x0b71, 0x0b82, 0x0011}, {0x0b83, 0x0b85, 0x0002}, {0x0b86, 0x0b8a, 0x0001}, {0x0b8e, 0x0b90, 0x0001},
	{0x0b92, 0x0b95, 0x0001}, {0x0b99, 0x0b9a, 0x0001}, {0x0b9c, 0x0b9e, 0x0002}, {0x0b9f, 0x0ba3, 0x0004},
	{0x0ba4, 0x0ba8, 0x0004}, {0x0ba9, 0x0baa, 0x0001}, {0x0bae, 0x0bb9, 0x0001}, {0x0bbe, 0x0bc2, 0x0001},
	{0x0bc6, 0x0bc8, 0x0001}, {0x0bca, 0x0bcd, 0x0001}, {0x0bd0, 0x0bd7, 0x0007}, {0x0be6, 0x0bef, 0x0001},
	{0x0c00, 0x0c0c, 0x0001}, {0x0c0e, 0x0c10, 0x0001}, {0x0c12, 0x0c28, 0x0001}, {0x0c2a, 0x0c39, 0x0001},
	{0x0c3c, 0x0c44, 0x0001}, {0x0c46, 0x0c48, 0x0001}, {0x0c4a, 0x0c4d, 0x0001}, {0x0c55, 0x0c56, 0x0001},
	{0x0c58, 0x0c5a, 0x0001}, {0x0c5c, 0x0c5d, 0x0001}, {0x0c60, 0x0c63, 0x0001}, {0x0c66, 0x0c6f, 0x0001},
	{0x0c80, 0x0c83, 0x0001}, {0x0c85, 0x0c8c, 0x0001}, {0x0c8e, 0x0c90, 0x0001}, {0x0c92, 0x0ca8, 0x0001},
	{0x0caa, 0x0cb3, 0x0001}, {0x0cb5, 0x0cb9, 0x0001}, {0x0cbc, 0x0cc4, 0x0001}, {0x0cc6, 0x0cc8, 0x0001},
	{0x0cca, 0x0ccd, 0x0001}, {0x0cd5, 0x0cd6, 0x0001}, {0x0cdc, 0x0cde, 0x0001}, {0x0ce0, 0x0ce3, 0x0001},
	{0x0ce6, 0x0cef, 0x0001}, {0x0cf1, 0x0cf3, 0x0001}, {0x0d00, 0x0d0c, 0x0001}, {0x0d0e, 0x0d10, 0x0001},
	{0x0d12, 0x0d44, 0x0001}, {0x0d46, 0x0d48, 0x0001}, {0x0d4a, 0x0d4e, 0x0001}, {0x0d54, 0x0d57, 0x0001},
	{0x0d5f, 0x0d63, 0x0001}, {0x0d66, 0x0d6f, 0x0001}, {0x0d7a, 0x0d7f, 0x0001}, {0x0d81, 0x0d83, 0x0001},
	{0x0d85, 0x0d96, 0x0001}, {0x0d9a, 0x0db1, 0x0001}, {0x0db3, 0x0dbb, 0x0001}, {0x0dbd, 0x0dc0, 0x0003},
	{0x0dc1, 0x0dc6, 0x0001}, {0x0dca, 0x0dcf, 0x0005}, {0x0dd0, 0x0dd4, 0x0001}, {0x0dd6, 0x0dd8, 0x0002},
	{0x0dd9, 0x0ddf, 0x0001}, {0x0de6, 0x0def, 0x0001}, {0x0df2, 0x0df3, 0x0001}, {0x0e01, 0x0e3a, 0x0001},
	{0x0e40, 0x0e4e, 0x0001}, {0x0e50, 0x0e59, 0x0001}, {0x0e81, 0x0e82, 0x0001}, {0x0e84, 0x0e86, 0x0002},
	{0x0e87, 0x0e8a, 0x0001}, {0x0e8c, 0x0ea3, 0x0001}, {0x0ea5, 0x0ea7, 0x0002}, {0x0ea8, 0x0ebd, 0x0001},
	{0x0ec0, 0x0ec4, 0x0001}, {0x0ec6, 0x0ec8, 0x0002}, {0x0ec9, 0x0ece, 0x0001}, {0x0ed0, 0x0ed9, 0x0001},
	{0x0edc, 0x0edf, 0x0001}, {0x0f00, 0x0f18, 0x0018}, {0x0f19, 0x0f20, 0x0007}, {0x0f21, 0x0f29, 0x0001},
	{0x0f35, 0x0f39, 0x0002}, {0x0f3e, 0x0f47, 0x0001}, {0x0f49, 0x0f6c, 0x0001}, {0x0f71, 0x0f84, 0x0001},
	{0x0f86, 0x0f97, 0x0001}, {0x0f99, 0x0fbc, 0x0001}, {0x0fc6, 0x1000, 0x003a}, {0x1001, 0x1049, 0x0001},
	{0x1050, 0x109d, 0x0001}, {0x10a0, 0x10c5, 0x0001}, {0x10c7, 0x10cd, 0x0006}, {0x10d0, 0x10fa, 0x0001},
	{0x10fc, 0x1248, 0x0001}, {0x124a, 0x124d, 0x0001}, {0x1250, 0x1256, 0x0001}, {0x1258, 0x125a, 0x0002},
	{0x125b, 0x125d, 0x0001}, {0x1260, 0x1288, 0x0001}, {0x128a, 0x128d, 0x0001}, {0x1290, 0x12b0, 0x0001},
	{0x12b2, 0x12b5, 0x0001}, {0x12b8, 0x12be, 0x0001}, {0x12c0, 0x12c2, 0x0002}, {0x12c3, 0x12c5, 0x0001},
	{0x12c8, 0x12d6, 0x0001}, {0x12d8, 0x1310, 0x0001}, {0x1312, 0x1315, 0x0001}, {0x1318, 0x135a, 0x0001},
	{0x135d, 0x135f, 0x0001}, {0x1369, 0x1371, 0x0001}, {0x1380, 0x138f, 0x0001}, {0x13a0, 0x13f5, 0x0001},
	{0x13f8, 0x13fd, 0x0001}, {0x1401, 0x166c, 0x0001}, {0x166f, 0x167f, 0x0001}, {0x1681, 0x169a, 0x0001},
	{0x16a0, 0x16ea, 0x0001}, {0x16ee, 0x16f8, 0x0001}, {0x1700, 0x1715, 0x0001}, {0x171f, 0x1734, 0x0001},
	{0x1740, 0x1753, 0x0001}, {0x1760, 0x176c, 0x0001}, {0x176e, 0x1770, 0x0001}, {0x1772, 0x1773, 0x0001},
	{0x1780, 0x17d3, 0x0001}, {0x17d7, 0x17dc, 0x0005}, {0x17dd, 0x17e0, 0x0003}, {0x17e1, 0x17e9, 0x0001},
	{0x180b, 0x180d, 0x0001}, {0x180f, 0x1819, 0x0001}, {0x1820, 0x1878, 0x0001}, {0x1880, 0x18aa, 0x0001},
	{0x18b0, 0x18f5, 0x0001}, {0x1900, 0x191e, 0x0001}, {0x1920, 0x192b, 0x0001}, {0x1930, 0x193b, 0x0001},
	{0x1946, 0x196d, 0x0001}, {0x1970, 0x1974, 0x0001}, {0x1980, 0x19ab, 0x0001}, {0x19b0, 0x19c9, 0x0001},
	{0x19d0, 0x19da, 0x0001}, {0x1a00, 0x1a1b, 0x0001}, {0x1a20, 0x1a5e, 0x0001}, {0x1a60, 0x1a7c, 0x0001},
	{0x1a7f, 0x1a89, 0x0001}, {0x1a90, 0x1a99, 0x0001}, {0x1aa7, 0x1ab0, 0x0009}, {0x1ab1, 0x1abd, 0x0001},
	{0x1abf, 0x1af0, 0x0001}, {0x1b00, 0x1b4c, 0x0001}, {0x1b50, 0x1b59, 0x0001}, {0x1b6b, 0x1b73, 0x0001},
	{0x1b80, 0x1bf3, 0x0001}, {0x1c00, 0x1c37, 0x0001}, {0x1c40, 0x1c49, 0x0001}, {0x1c4d, 0x1c7d, 0x0001},
	{0x1c80, 0x1c8a, 0x0001}, {0x1c90, 0x1cba, 0x0001}, {0x1cbd, 0x1cbf, 0x0001}, {0x1cd0, 0x1cd2, 0x0001},
	{0x1cd4, 0x1cfa, 0x0001}, {0x1d00, 0x1f15, 0x0001}, {0x1f18, 0x1f1d, 0x0001}, {0x1f20, 0x1f45, 0x0001},
	{0x1f48, 0x1f4d, 0x0001}, {0x1f50, 0x1f57, 0x0001}, {0x1f59, 0x1f5f, 0x0002}, {0x1f60, 0x1f7d, 0x0001},
	{0x1f80, 0x1fb4, 0x0001}, {0x1fb6, 0x1fbc, 0x0001}, {0x1fbe, 0x1fc2, 0x0004}, {0x1fc3, 0x1fc4, 0x0001},
	{0x1fc6, 0x1fcc, 0x0001}, {0x1fd0, 0x1fd3, 0x0001}, {0x1fd6, 0x1fdb, 0x0001}, {0x1fe0, 0x1fec, 0x0001},
	{0x1ff2, 0x1ff4, 0x0001}, {0x1ff6, 0x1ffc, 0x0001}, {0x200c, 0x200d, 0x0001}, {0x203f, 0x2040, 0x0001},
	{0x2054, 0x2071, 0x001d}, {0x207f, 0x208f, 0x0010}, {0x2090, 0x209f, 0x0001}, {0x20d0, 0x20dc, 0x0001},
	{0x20e1, 0x20e5, 0x0004}, {0x20e6, 0x20f0, 0x0001}, {0x2102, 0x2107, 0x0005}, {0x210a, 0x2113, 0x0001},
	{0x2115, 0x2118, 0x0003}, {0x2119, 0x211d, 0x0001}, {0x2124, 0x212a, 0x0002}, {0x212b, 0x2139, 0x0001},
	{0x213c, 0x213f, 0x0001}, {0x2145, 0x2149, 0x0001}, {0x214e, 0x2160, 0x0012}, {0x2161, 0x2188, 0x0001},
	{0x2c00, 0x2ce4, 0x0001}, {0x2ceb, 0x2cf3, 0x0001}, {0x2d00, 0x2d25, 0x0001}, {0x2d27, 0x2d2d, 0x0006},
	{0x2d30, 0x2d67, 0x0001}, {0x2d6f, 0x2d7f, 0x0010}, {0x2d80, 0x2d96, 0x0001}, {0x2da0, 0x2da6, 0x0001},
	{0x2da8, 0x2dae, 0x0001}, {0x2db0, 0x2db6, 0x0001}, {0x2db8, 0x2dbe, 0x0001}, {0x2dc0, 0x2dc6, 0x0001},
	{0x2dc8, 0x2dce, 0x0001}, {0x2dd0, 0x2dd6, 0x0001}, {0x2dd8, 0x2dde, 0x0001}, {0x2de0, 0x2dff, 0x0001},
	{0x3005, 0x3007, 0x0001}, {0x3021, 0x302f, 0x0001}, {0x3031, 0x3035, 0x0001}, {0x3038, 0x303c, 0x0001},
	{0x3041, 0x3096, 0x0001}, {0x3099, 0x309a, 0x0001}, {0x309d, 0x309f, 0x0001}, {0x30a1, 0x30ff, 0x0001},
	{0x3105, 0x312f, 0x0001}, {0x3131, 0x318e, 0x0001}, {0x31a0, 0x31bf, 0x0001}, {0x31f0, 0x31ff, 0x0001},
	{0x3400, 0x4dbf, 0x0001}, {0x4e00, 0xa48c, 0x0001}, {0xa4d0, 0xa4fd, 0x0001}, {0xa500, 0xa60c, 0x0001},
	{0xa610, 0xa62b, 0x0001}, {0xa640, 0xa66f, 0x0001}, {0xa674, 0xa67d, 0x0001}, {0xa67f, 0xa6f1, 0x0001},
	{0xa717, 0xa71f, 0x0001}, {0xa722, 0xa788, 0x0001}, {0xa78b, 0xa7dd, 0x0001}, {0xa7e2, 0xa7f1, 0x000f},
	{0xa7f2, 0xa827, 0x0001}, {0xa82c, 0xa840, 0x0014}, {0xa841, 0xa873, 0x0001}, {0xa880, 0xa8c5, 0x0001},
	{0xa8d0, 0xa8d9, 0x0001}, {0xa8e0, 0xa8f7, 0x0001}, {0xa8fb, 0xa8fd, 0x0002}, {0xa8fe, 0xa92d, 0x0001},
	{0xa930, 0xa953, 0x0001}, {0xa960, 0xa97c, 0x0001}, {0xa980, 0xa9c0, 0x0001}, {0xa9cf, 0xa9d9, 0x0001},
	{0xa9e0, 0xa9fe, 0x0001}, {0xaa00, 0xaa36, 0x0001}, {0xaa40, 0xaa4d, 0x0001}, {0xaa50, 0xaa59, 0x0001},
	{0xaa60, 0xaa76, 0x0001}, {0xaa7a, 0xaac2, 0x0001}, {0xaadb, 0xaadd, 0x0001}, {0xaae0, 0xaaef, 0x0001},
	{0xaaf2, 0xaaf6, 0x0001}, {0xab01, 0xab06, 0x0001}, {0xab09, 0xab0e, 0x0001}, {0xab11, 0xab16, 0x0001},
	{0xab20, 0xab26, 0x0001}, {0xab28, 0xab2e, 0x0001}, {0xab30, 0xab5a, 0x0001}, {0xab5c, 0xab69, 0x0001},
	{0xab6c, 0xab6d, 0x0001}, {0xab70, 0xabea, 0x0001}, {0xabec, 0xabed, 0x0001}, {0xabf0, 0xabf9, 0x0001},
	{0xac00, 0xd7a3, 0x0001}, {0xd7b0, 0xd7c6, 0x0001}, {0xd7cb, 0xd7fb, 0x0001}, {0xf900, 0xfa6d, 0x0001},
	{0xfa70, 0xfad9, 0x0001}, {0xfb00, 0xfb06, 0x0001}, {0xfb13, 0xfb17, 0x0001}, {0xfb1d, 0xfb28, 0x0001},
	{0xfb2a, 0xfb36, 0x0001}, {0xfb38, 0xfb3c, 0x0001}, {0xfb3e, 0xfb40, 0x0002}, {0xfb41, 0xfb43, 0x0002},
	{0xfb44, 0xfb46, 0x0002}, {0xfb47, 0xfbb1, 0x0001}, {0xfbd3, 0xfc5d, 0x0001}, {0xfc64, 0xfd3d, 0x0001},
	{0xfd50, 0xfd8f, 0x0001}, {0xfd92, 0xfdc7, 0x0001}, {0xfdf0, 0xfdf9, 0x0001}, {0xfe00, 0xfe0f, 0x0001},
	{0xfe20, 0xfe2f, 0x0001}, {0xfe33, 0xfe34, 0x0001}, {0xfe4d, 0xfe4f, 0x0001}, {0xfe71, 0xfe73, 0x0002},
	{0xfe77, 0xfe7f, 0x0002}, {0xfe80, 0xfefc, 0x0001}, {0xff10, 0xff19, 0x0001}, {0xff21, 0xff3a, 0x0001},
	{0xff3f, 0xff41, 0x0002}, {0xff42, 0xff5a, 0x0001}, {0xff65, 0xffbe, 0x0001}, {0xffc2, 0xffc7, 0x0001},
	{0xffca, 0xffcf, 0x0001}, {0xffd2, 0xffd7, 0x0001}, {0xffda, 0xffdc, 0x0001}
};


//
//	xidContinue32
//

#if defined(IMGUI_USE_WCHAR32)

static StrideRange32 xidContinue32[] = {
	{0x10000, 0x1000b, 0x0001}, {0x1000d, 0x10026, 0x0001}, {0x10028, 0x1003a, 0x0001}, {0x1003c, 0x1003d, 0x0001},
	{0x1003f, 0x1004d, 0x0001}, {0x10050, 0x1005d, 0x0001}, {0x10080, 0x100fa, 0x0001}, {0x10140, 0x10174, 0x0001},
	{0x101fd, 0x10280, 0x0083}, {0x10281, 0x1029c, 0x0001}, {0x102a0, 0x102d0, 0x0001}, {0x102e0, 0x10300, 0x0020},
	{0x10301, 0x1031f, 0x0001}, {0x1032d, 0x1034a, 0x0001}, {0x10350, 0x1037a, 0x0001}, {0x10380, 0x1039d, 0x0001},
	{0x103a0, 0x103c3, 0x0001}, {0x103c8, 0x103cf, 0x0001}, {0x103d1, 0x103d5, 0x0001}, {0x10400, 0x1049d, 0x0001},
	{0x104a0, 0x104a9, 0x0001}, {0x104b0, 0x104d3, 0x0001}, {0x104d8, 0x104fb, 0x0001}, {0x10500, 0x10527, 0x0001},
	{0x10530, 0x10563, 0x0001}, {0x10570, 0x1057a, 0x0001}, {0x1057c, 0x1058a, 0x0001}, {0x1058c, 0x10592, 0x0001},
	{0x10594, 0x10595, 0x0001}, {0x10597, 0x105a1, 0x0001}, {0x105a3, 0x105b1, 0x0001}, {0x105b3, 0x105b9, 0x0001},
	{0x105bb, 0x105bc, 0x0001}, {0x105c0, 0x105f3, 0x0001}, {0x10600, 0x10736, 0x0001}, {0x10740, 0x10755, 0x0001},
	{0x10760, 0x10767, 0x0001}, {0x10780, 0x10785, 0x0001}, {0x10787, 0x107b0, 0x0001}, {0x107b2, 0x107bf, 0x0001},
	{0x10800, 0x10805, 0x0001}, {0x10808, 0x1080a, 0x0002}, {0x1080b, 0x10835, 0x0001}, {0x10837, 0x10838, 0x0001},
	{0x1083c, 0x1083f, 0x0003}, {0x10840, 0x10855, 0x0001}, {0x10860, 0x10876, 0x0001}, {0x10880, 0x1089e, 0x0001},
	{0x108e0, 0x108f2, 0x0001}, {0x108f4, 0x108f5, 0x0001}, {0x10900, 0x10915, 0x0001}, {0x10920, 0x10939, 0x0001},
	{0x10940, 0x10959, 0x0001}, {0x10980, 0x109b7, 0x0001}, {0x109be, 0x109bf, 0x0001}, {0x10a00, 0x10a03, 0x0001},
	{0x10a05, 0x10a06, 0x0001}, {0x10a0c, 0x10a13, 0x0001}, {0x10a15, 0x10a17, 0x0001}, {0x10a19, 0x10a35, 0x0001},
	{0x10a38, 0x10a3a, 0x0001}, {0x10a3f, 0x10a60, 0x0021}, {0x10a61, 0x10a7c, 0x0001}, {0x10a80, 0x10a9c, 0x0001},
	{0x10ac0, 0x10ac7, 0x0001}, {0x10ac9, 0x10ae6, 0x0001}, {0x10b00, 0x10b35, 0x0001}, {0x10b40, 0x10b55, 0x0001},
	{0x10b60, 0x10b72, 0x0001}, {0x10b80, 0x10b91, 0x0001}, {0x10c00, 0x10c48, 0x0001}, {0x10c80, 0x10cb2, 0x0001},
	{0x10cc0, 0x10cf2, 0x0001}, {0x10d00, 0x10d27, 0x0001}, {0x10d30, 0x10d39, 0x0001}, {0x10d40, 0x10d65, 0x0001},
	{0x10d69, 0x10d6d, 0x0001}, {0x10d6f, 0x10d85, 0x0001}, {0x10e80, 0x10ea9, 0x0001}, {0x10eab, 0x10eac, 0x0001},
	{0x10eb0, 0x10eb1, 0x0001}, {0x10ec2, 0x10ec7, 0x0001}, {0x10ecb, 0x10ecf, 0x0001}, {0x10ed9, 0x10eee, 0x0001},
	{0x10ef0, 0x10f1c, 0x0001}, {0x10f27, 0x10f30, 0x0009}, {0x10f31, 0x10f50, 0x0001}, {0x10f70, 0x10f85, 0x0001},
	{0x10fb0, 0x10fc4, 0x0001}, {0x10fe0, 0x10ff6, 0x0001}, {0x11000, 0x11046, 0x0001}, {0x11066, 0x11075, 0x0001},
	{0x1107f, 0x110ba, 0x0001}, {0x110c2, 0x110d0, 0x000e}, {0x110d1, 0x110e8, 0x0001}, {0x110f0, 0x110f9, 0x0001},
	{0x11100, 0x11134, 0x0001}, {0x11136, 0x1113f, 0x0001}, {0x11144, 0x11147, 0x0001}, {0x11150, 0x11173, 0x0001},
	{0x11176, 0x11180, 0x000a}, {0x11181, 0x111c4, 0x0001}, {0x111c9, 0x111cc, 0x0001}, {0x111ce, 0x111da, 0x0001},
	{0x111dc, 0x11200, 0x0024}, {0x11201, 0x11211, 0x0001}, {0x11213, 0x11237, 0x0001}, {0x1123e, 0x11241, 0x0001},
	{0x11280, 0x11286, 0x0001}, {0x11288, 0x1128a, 0x0002}, {0x1128b, 0x1128d, 0x0001}, {0x1128f, 0x1129d, 0x0001},
	{0x1129f, 0x112a8, 0x0001}, {0x112b0, 0x112ea, 0x0001}, {0x112f0, 0x112f9, 0x0001}, {0x11300, 0x11303, 0x0001},
	{0x11305, 0x1130c, 0x0001}, {0x1130f, 0x11310, 0x0001}, {0x11313, 0x11328, 0x0001}, {0x1132a, 0x11330, 0x0001},
	{0x11332, 0x11333, 0x0001}, {0x11335, 0x11339, 0x0001}, {0x1133b, 0x11344, 0x0001}, {0x11347, 0x11348, 0x0001},
	{0x1134b, 0x1134d, 0x0001}, {0x11350, 0x11357, 0x0007}, {0x1135d, 0x11363, 0x0001}, {0x11366, 0x1136c, 0x0001},
	{0x11370, 0x11374, 0x0001}, {0x11380, 0x11389, 0x0001}, {0x1138b, 0x1138e, 0x0003}, {0x11390, 0x113b5, 0x0001},
	{0x113b7, 0x113c0, 0x0001}, {0x113c2, 0x113c5, 0x0003}, {0x113c7, 0x113ca, 0x0001}, {0x113cc, 0x113d3, 0x0001},
	{0x113e1, 0x113e2, 0x0001}, {0x11400, 0x1144a, 0x0001}, {0x11450, 0x11459, 0x0001}, {0x1145e, 0x11461, 0x0001},
	{0x11480, 0x114c5, 0x0001}, {0x114c7, 0x114d0, 0x0009}, {0x114d1, 0x114d9, 0x0001}, {0x11580, 0x115b5, 0x0001},
	{0x115b8, 0x115c0, 0x0001}, {0x115d8, 0x115dd, 0x0001}, {0x11600, 0x11640, 0x0001}, {0x11644, 0x11650, 0x000c},
	{0x11651, 0x11659, 0x0001}, {0x11680, 0x116b8, 0x0001}, {0x116c0, 0x116c9, 0x0001}, {0x116d0, 0x116e3, 0x0001},
	{0x11700, 0x1171a, 0x0001}, {0x1171d, 0x1172b, 0x0001}, {0x11730, 0x11739, 0x0001}, {0x11740, 0x11746, 0x0001},
	{0x11800, 0x1183a, 0x0001}, {0x118a0, 0x118e9, 0x0001}, {0x118ff, 0x11906, 0x0001}, {0x11909, 0x1190c, 0x0003},
	{0x1190d, 0x11913, 0x0001}, {0x11915, 0x11916, 0x0001}, {0x11918, 0x11935, 0x0001}, {0x11937, 0x11938, 0x0001},
	{0x1193b, 0x11943, 0x0001}, {0x11950, 0x11959, 0x0001}, {0x119a0, 0x119a7, 0x0001}, {0x119aa, 0x119d7, 0x0001},
	{0x119da, 0x119e1, 0x0001}, {0x119e3, 0x119e4, 0x0001}, {0x11a00, 0x11a3e, 0x0001}, {0x11a47, 0x11a50, 0x0009},
	{0x11a51, 0x11a99, 0x0001}, {0x11a9d, 0x11ab0, 0x0013}, {0x11ab1, 0x11af8, 0x0001}, {0x11b0a, 0x11b60, 0x0056},
	{0x11b61, 0x11b67, 0x0001}, {0x11bc0, 0x11be0, 0x0001}, {0x11bf0, 0x11bf9, 0x0001}, {0x11c00, 0x11c08, 0x0001},
	{0x11c0a, 0x11c36, 0x0001}, {0x11c38, 0x11c40, 0x0001}, {0x11c50, 0x11c59, 0x0001}, {0x11c72, 0x11c8f, 0x0001},
	{0x11c92, 0x11ca7, 0x0001}, {0x11ca9, 0x11cb6, 0x0001}, {0x11d00, 0x11d06, 0x0001}, {0x11d08, 0x11d09, 0x0001},
	{0x11d0b, 0x11d36, 0x0001}, {0x11d3a, 0x11d3c, 0x0002}, {0x11d3d, 0x11d3f, 0x0002}, {0x11d40, 0x11d47, 0x0001},
	{0x11d50, 0x11d59, 0x0001}, {0x11d60, 0x11d65, 0x0001}, {0x11d67, 0x11d68, 0x0001}, {0x11d6a, 0x11d8e, 0x0001},
	{0x11d90, 0x11d91, 0x0001}, {0x11d93, 0x11d98, 0x0001}, {0x11da0, 0x11da9, 0x0001}, {0x11db0, 0x11ddb, 0x0001},
	{0x11de0, 0x11de9, 0x0001}, {0x11df0, 0x11df1, 0x0001}, {0x11ee0, 0x11ef6, 0x0001}, {0x11f00, 0x11f10, 0x0001},
	{0x11f12, 0x11f3a, 0x0001}, {0x11f3e, 0x11f42, 0x0001}, {0x11f50, 0x11f5a, 0x0001}, {0x11fb0, 0x12000, 0x0050},
	{0x12001, 0x12399, 0x0001}, {0x12400, 0x1246f, 0x0001}, {0x12475, 0x12543, 0x0001}, {0x12550, 0x12686, 0x0001},
	{0x12f90, 0x12ff0, 0x0001}, {0x13000, 0x1342f, 0x0001}, {0x13440, 0x13455, 0x0001}, {0x13460, 0x143fa, 0x0001},
	{0x14400, 0x14646, 0x0001}, {0x16100, 0x16139, 0x0001}, {0x16800, 0x16a38, 0x0001}, {0x16a40, 0x16a5e, 0x0001},
	{0x16a60, 0x16a69, 0x0001}, {0x16a70, 0x16abe, 0x0001}, {0x16ac0, 0x16ac9, 0x0001}, {0x16ad0, 0x16aed, 0x0001},
	{0x16af0, 0x16af4, 0x0001}, {0x16b00, 0x16b36, 0x0001}, {0x16b40, 0x16b43, 0x0001}, {0x16b50, 0x16b59, 0x0001},
	{0x16b63, 0x16b77, 0x0001}, {0x16b7d, 0x16b8f, 0x0001}, {0x16d40, 0x16d6c, 0x0001}, {0x16d70, 0x16d79, 0x0001},
	{0x16e40, 0x16e7f, 0x0001}, {0x16ea0, 0x16eb8, 0x0001}, {0x16ebb, 0x16ed3, 0x0001}, {0x16f00, 0x16f4a, 0x0001},
	{0x16f4f, 0x16f87, 0x0001}, {0x16f8f, 0x16f9f, 0x0001}, {0x16fe0, 0x16fe1, 0x0001}, {0x16fe3, 0x16fe4, 0x0001},
	{0x16ff0, 0x16ff6, 0x0001}, {0x17000, 0x18cda, 0x0001}, {0x18cff, 0x18d20, 0x0001}, {0x18d80, 0x18df2, 0x0001},
	{0x18e00, 0x19191, 0x0001}, {0x191a0, 0x191d2, 0x0001}, {0x1aff0, 0x1aff3, 0x0001}, {0x1aff5, 0x1affb, 0x0001},
	{0x1affd, 0x1affe, 0x0001}, {0x1b000, 0x1b128, 0x0001}, {0x1b132, 0x1b150, 0x001e}, {0x1b151, 0x1b152, 0x0001},
	{0x1b155, 0x1b164, 0x000f}, {0x1b165, 0x1b168, 0x0001}, {0x1b170, 0x1b2fb, 0x0001}, {0x1bc00, 0x1bc6a, 0x0001},
	{0x1bc70, 0x1bc7c, 0x0001}, {0x1bc80, 0x1bc88, 0x0001}, {0x1bc90, 0x1bc99, 0x0001}, {0x1bc9d, 0x1bc9e, 0x0001},
	{0x1ccf0, 0x1ccf9, 0x0001}, {0x1cf00, 0x1cf2d, 0x0001}, {0x1cf30, 0x1cf46, 0x0001}, {0x1d127, 0x1d128, 0x0001},
	{0x1d165, 0x1d169, 0x0001}, {0x1d16d, 0x1d172, 0x0001}, {0x1d17b, 0x1d182, 0x0001}, {0x1d185, 0x1d18b, 0x0001},
	{0x1d1aa, 0x1d1ad, 0x0001}, {0x1d242, 0x1d244, 0x0001}, {0x1d250, 0x1d252, 0x0001}, {0x1d25b, 0x1d25c, 0x0001},
	{0x1d25f, 0x1d280, 0x0021}, {0x1d281, 0x1d400, 0x017f}, {0x1d401, 0x1d454, 0x0001}, {0x1d456, 0x1d49c, 0x0001},
	{0x1d49e, 0x1d49f, 0x0001}, {0x1d4a2, 0x1d4a5, 0x0003}, {0x1d4a6, 0x1d4a9, 0x0003}, {0x1d4aa, 0x1d4ac, 0x0001},
	{0x1d4ae, 0x1d4b9, 0x0001}, {0x1d4bb, 0x1d4bd, 0x0002}, {0x1d4be, 0x1d4c3, 0x0001}, {0x1d4c5, 0x1d505, 0x0001},
	{0x1d507, 0x1d50a, 0x0001}, {0x1d50d, 0x1d514, 0x0001}, {0x1d516, 0x1d51c, 0x0001}, {0x1d51e, 0x1d539, 0x0001},
	{0x1d53b, 0x1d53e, 0x0001}, {0x1d540, 0x1d544, 0x0001}, {0x1d546, 0x1d54a, 0x0004}, {0x1d54b, 0x1d550, 0x0001},
	{0x1d552, 0x1d6a6, 0x0001}, {0x1d6a8, 0x1d6c0, 0x0001}, {0x1d6c2, 0x1d6da, 0x0001}, {0x1d6dc, 0x1d6fa, 0x0001},
	{0x1d6fc, 0x1d714, 0x0001}, {0x1d716, 0x1d734, 0x0001}, {0x1d736, 0x1d74e, 0x0001}, {0x1d750, 0x1d76e, 0x0001},
	{0x1d770, 0x1d788, 0x0001}, {0x1d78a, 0x1d7a8, 0x0001}, {0x1d7aa, 0x1d7c2, 0x0001}, {0x1d7c4, 0x1d7cb, 0x0001},
	{0x1d7ce, 0x1d7ff, 0x0001}, {0x1da00, 0x1da36, 0x0001}, {0x1da3b, 0x1da6c, 0x0001}, {0x1da75, 0x1da84, 0x000f},
	{0x1da9b, 0x1da9f, 0x0001}, {0x1daa1, 0x1daaf, 0x0001}, {0x1df00, 0x1df81, 0x0001}, {0x1df90, 0x1df96, 0x0001},
	{0x1dfcd, 0x1e006, 0x0001}, {0x1e008, 0x1e018, 0x0001}, {0x1e01b, 0x1e021, 0x0001}, {0x1e023, 0x1e024, 0x0001},
	{0x1e026, 0x1e02a, 0x0001}, {0x1e030, 0x1e06d, 0x0001}, {0x1e08f, 0x1e100, 0x0071}, {0x1e101, 0x1e12c, 0x0001},
	{0x1e130, 0x1e13d, 0x0001}, {0x1e140, 0x1e149, 0x0001}, {0x1e14e, 0x1e290, 0x0142}, {0x1e291, 0x1e2ae, 0x0001},
	{0x1e2c0, 0x1e2f9, 0x0001}, {0x1e4d0, 0x1e4f9, 0x0001}, {0x1e5d0, 0x1e5fa, 0x0001}, {0x1e6c0, 0x1e6de, 0x0001},
	{0x1e6e0, 0x1e6f5, 0x0001}, {0x1e6fe, 0x1e6ff, 0x0001}, {0x1e7e0, 0x1e7e6, 0x0001}, {0x1e7e8, 0x1e7eb, 0x0001},
	{0x1e7ed, 0x1e7ee, 0x0001}, {0x1e7f0, 0x1e7fe, 0x0001}, {0x1e800, 0x1e8c4, 0x0001}, {0x1e8d0, 0x1e8d6, 0x0001},
	{0x1e900, 0x1e94b, 0x0001}, {0x1e950, 0x1e959, 0x0001}, {0x1ee00, 0x1ee03, 0x0001}, {0x1ee05, 0x1ee1f, 0x0001},
	{0x1ee21, 0x1ee22, 0x0001}, {0x1ee24, 0x1ee27, 0x0003}, {0x1ee29, 0x1ee32, 0x0001}, {0x1ee34, 0x1ee37, 0x0001},
	{0x1ee39, 0x1ee3b, 0x0002}, {0x1ee42, 0x1ee47, 0x0005}, {0x1ee49, 0x1ee4d, 0x0002}, {0x1ee4e, 0x1ee4f, 0x0001},
	{0x1ee51, 0x1ee52, 0x0001}, {0x1ee54, 0x1ee57, 0x0003}, {0x1ee59, 0x1ee61, 0x0002}, {0x1ee62, 0x1ee64, 0x0002},
	{0x1ee67, 0x1ee6a, 0x0001}, {0x1ee6c, 0x1ee72, 0x0001}, {0x1ee74, 0x1ee77, 0x0001}, {0x1ee79, 0x1ee7c, 0x0001},
	{0x1ee7e, 0x1ee80, 0x0002}, {0x1ee81, 0x1ee89, 0x0001}, {0x1ee8b, 0x1ee9b, 0x0001}, {0x1eea1, 0x1eea3, 0x0001},
	{0x1eea5, 0x1eea9, 0x0001}, {0x1eeab, 0x1eebb, 0x0001}, {0x1fbf0, 0x1fbf9, 0x0001}, {0x20000, 0x2a6df, 0x0001},
	{0x2a700, 0x2b81e, 0x0001}, {0x2b820, 0x2cead, 0x0001}, {0x2ceb0, 0x2ebe0, 0x0001}, {0x2ebf0, 0x2ee5d, 0x0001},
	{0x2f800, 0x2fa1d, 0x0001}, {0x30000, 0x3134a, 0x0001}, {0x31350, 0x33479, 0x0001}, {0x3d000, 0x3fc3f, 0x0001},
	{0xe0100, 0xe01ef, 0x0001}
};

#endif


//
//	eastAsian16
//

static Range16 eastAsian16[] = {
	{0x1100, 0x115f}, {0x20a9, 0x20a9}, {0x231a, 0x231b}, {0x2329, 0x232a}, {0x23e9, 0x23ec}, {0x23f0, 0x23f0},
	{0x23f3, 0x23f3}, {0x25fd, 0x25fe}, {0x2614, 0x2615}, {0x2630, 0x2637}, {0x2648, 0x2653}, {0x267f, 0x267f},
	{0x268a, 0x268f}, {0x2693, 0x2693}, {0x26a1, 0x26a1}, {0x26aa, 0x26ab}, {0x26bd, 0x26be}, {0x26c4, 0x26c5},
	{0x26ce, 0x26ce}, {0x26d4, 0x26d4}, {0x26ea, 0x26ea}, {0x26f2, 0x26f3}, {0x26f5, 0x26f5}, {0x26fa, 0x26fa},
	{0x26fd, 0x26fd}, {0x2705, 0x2705}, {0x270a, 0x270b}, {0x2728, 0x2728}, {0x274c, 0x274c}, {0x274e, 0x274e},
	{0x2753, 0x2755}, {0x2757, 0x2757}, {0x2795, 0x2797}, {0x27b0, 0x27b0}, {0x27bf, 0x27bf}, {0x2b1b, 0x2b1c},
	{0x2b50, 0x2b50}, {0x2b55, 0x2b55}, {0x2e80, 0x2e99}, {0x2e9b, 0x2ef3}, {0x2f00, 0x2fd5}, {0x2ff0, 0x2fff},
	{0x3000, 0x3000}, {0x3001, 0x303e}, {0x3041, 0x3096}, {0x3099, 0x30ff}, {0x3105, 0x312f}, {0x3131, 0x318e},
	{0x3190, 0x31e5}, {0x31ef, 0x321e}, {0x3220, 0x3247}, {0x3250, 0xa48c}, {0xa490, 0xa4c6}, {0xa960, 0xa97c},
	{0xac00, 0xd7a3}, {0xf900, 0xfaff}, {0xfe10, 0xfe19}, {0xfe30, 0xfe52}, {0xfe54, 0xfe66}, {0xfe68, 0xfe6b},
	{0xff01, 0xff60}, {0xff61, 0xffbe}, {0xffc2, 0xffc7}, {0xffca, 0xffcf}, {0xffd2, 0xffd7}, {0xffda, 0xffdc},
	{0xffe0, 0xffe6}, {0xffe8, 0xffee}
};


//
//	eastAsian32
//

#if defined(IMGUI_USE_WCHAR32)

static Range32 eastAsian32[] = {
	{0x16fe0, 0x16fe4}, {0x16ff0, 0x16ff6}, {0x17000, 0x18cda}, {0x18cff, 0x18d20}, {0x18d80, 0x18df2}, {0x18e00, 0x19191},
	{0x191a0, 0x191d2}, {0x1aff0, 0x1aff3}, {0x1aff5, 0x1affb}, {0x1affd, 0x1affe}, {0x1b000, 0x1b128}, {0x1b132, 0x1b132},
	{0x1b150, 0x1b152}, {0x1b155, 0x1b155}, {0x1b164, 0x1b168}, {0x1b170, 0x1b2fb}, {0x1d300, 0x1d356}, {0x1d360, 0x1d376},
	{0x1f004, 0x1f004}, {0x1f0cf, 0x1f0cf}, {0x1f18e, 0x1f18e}, {0x1f191, 0x1f19a}, {0x1f1ae, 0x1f1ae}, {0x1f200, 0x1f202},
	{0x1f210, 0x1f23b}, {0x1f240, 0x1f248}, {0x1f250, 0x1f251}, {0x1f260, 0x1f265}, {0x1f300, 0x1f320}, {0x1f32d, 0x1f335},
	{0x1f337, 0x1f37c}, {0x1f37e, 0x1f393}, {0x1f3a0, 0x1f3ca}, {0x1f3cf, 0x1f3d3}, {0x1f3e0, 0x1f3f0}, {0x1f3f4, 0x1f3f4},
	{0x1f3f8, 0x1f43e}, {0x1f440, 0x1f440}, {0x1f442, 0x1f4fc}, {0x1f4ff, 0x1f53d}, {0x1f54b, 0x1f54e}, {0x1f550, 0x1f567},
	{0x1f57a, 0x1f57a}, {0x1f595, 0x1f596}, {0x1f5a4, 0x1f5a4}, {0x1f5fb, 0x1f64f}, {0x1f680, 0x1f6c5}, {0x1f6cc, 0x1f6cc},
	{0x1f6d0, 0x1f6d2}, {0x1f6d5, 0x1f6d9}, {0x1f6dc, 0x1f6df}, {0x1f6eb, 0x1f6ec}, {0x1f6f4, 0x1f6fc}, {0x1f7da, 0x1f7da},
	{0x1f7e0, 0x1f7eb}, {0x1f7f0, 0x1f7f0}, {0x1f90c, 0x1f93a}, {0x1f93c, 0x1f945}, {0x1f947, 0x1f9ff}, {0x1fa70, 0x1fa7c},
	{0x1fa80, 0x1fac6}, {0x1fac8, 0x1fac8}, {0x1facc, 0x1fadd}, {0x1fadf, 0x1faeb}, {0x1faef, 0x1fafa}, {0x20000, 0x2fffd},
	{0x30000, 0x3fffd}
};

#endif


//
//	wideGlyph16
//

static Range16 wideGlyph16[] = {
	{0x1100, 0x115f}, {0x231a, 0x231b}, {0x2329, 0x232a}, {0x23e9, 0x23ec}, {0x23f0, 0x23f0}, {0x23f3, 0x23f3},
	{0x25fd, 0x25fe}, {0x2614, 0x2615}, {0x2630, 0x2637}, {0x2648, 0x2653}, {0x267f, 0x267f}, {0x268a, 0x268f},
	{0x2693, 0x2693}, {0x26a1, 0x26a1}, {0x26aa, 0x26ab}, {0x26bd, 0x26be}, {0x26c4, 0x26c5}, {0x26ce, 0x26ce},
	{0x26d4, 0x26d4}, {0x26ea, 0x26ea}, {0x26f2, 0x26f3}, {0x26f5, 0x26f5}, {0x26fa, 0x26fa}, {0x26fd, 0x26fd},
	{0x2705, 0x2705}, {0x270a, 0x270b}, {0x2728, 0x2728}, {0x274c, 0x274c}, {0x274e, 0x274e}, {0x2753, 0x2755},
	{0x2757, 0x2757}, {0x2795, 0x2797}, {0x27b0, 0x27b0}, {0x27bf, 0x27bf}, {0x2b1b, 0x2b1c}, {0x2b50, 0x2b50},
	{0x2b55, 0x2b55}, {0x2e80, 0x2e99}, {0x2e9b, 0x2ef3}, {0x2f00, 0x2fd5}, {0x2ff0, 0x2fff}, {0x3001, 0x303e},
	{0x3041, 0x3096}, {0x3099, 0x30ff}, {0x3105, 0x312f}, {0x3131, 0x318e}, {0x3190, 0x31e5}, {0x31ef, 0x321e},
	{0x3220, 0x3247}, {0x3250, 0xa48c}, {0xa490, 0xa4c6}, {0xa960, 0xa97c}, {0xac00, 0xd7a3}, {0xf900, 0xfaff},
	{0xfe10, 0xfe19}, {0xfe30, 0xfe52}, {0xfe54, 0xfe66}, {0xfe68, 0xfe6b}
};


//
//	wideGlyph32
//

#if defined(IMGUI_USE_WCHAR32)

static Range32 wideGlyph32[] = {
	{0x16fe0, 0x16fe4}, {0x16ff0, 0x16ff6}, {0x17000, 0x18cda}, {0x18cff, 0x18d20}, {0x18d80, 0x18df2}, {0x18e00, 0x19191},
	{0x191a0, 0x191d2}, {0x1aff0, 0x1aff3}, {0x1aff5, 0x1affb}, {0x1affd, 0x1affe}, {0x1b000, 0x1b128}, {0x1b132, 0x1b132},
	{0x1b150, 0x1b152}, {0x1b155, 0x1b155}, {0x1b164, 0x1b168}, {0x1b170, 0x1b2fb}, {0x1d300, 0x1d356}, {0x1d360, 0x1d376},
	{0x1f004, 0x1f004}, {0x1f0cf, 0x1f0cf}, {0x1f18e, 0x1f18e}, {0x1f191, 0x1f19a}, {0x1f1ae, 0x1f1ae}, {0x1f200, 0x1f202},
	{0x1f210, 0x1f23b}, {0x1f240, 0x1f248}, {0x1f250, 0x1f251}, {0x1f260, 0x1f265}, {0x1f300, 0x1f320}, {0x1f32d, 0x1f335},
	{0x1f337, 0x1f37c}, {0x1f37e, 0x1f393}, {0x1f3a0, 0x1f3ca}, {0x1f3cf, 0x1f3d3}, {0x1f3e0, 0x1f3f0}, {0x1f3f4, 0x1f3f4},
	{0x1f3f8, 0x1f43e}, {0x1f440, 0x1f440}, {0x1f442, 0x1f4fc}, {0x1f4ff, 0x1f53d}, {0x1f54b, 0x1f54e}, {0x1f550, 0x1f567},
	{0x1f57a, 0x1f57a}, {0x1f595, 0x1f596}, {0x1f5a4, 0x1f5a4}, {0x1f5fb, 0x1f64f}, {0x1f680, 0x1f6c5}, {0x1f6cc, 0x1f6cc},
	{0x1f6d0, 0x1f6d2}, {0x1f6d5, 0x1f6d9}, {0x1f6dc, 0x1f6df}, {0x1f6eb, 0x1f6ec}, {0x1f6f4, 0x1f6fc}, {0x1f7da, 0x1f7da},
	{0x1f7e0, 0x1f7eb}, {0x1f7f0, 0x1f7f0}, {0x1f90c, 0x1f93a}, {0x1f93c, 0x1f945}, {0x1f947, 0x1f9ff}, {0x1fa70, 0x1fa7c},
	{0x1fa80, 0x1fac6}, {0x1fac8, 0x1fac8}, {0x1facc, 0x1fadd}, {0x1fadf, 0x1faeb}, {0x1faef, 0x1fafa}, {0x20000, 0x2fffd},
	{0x30000, 0x3fffd}
};

#endif


//
//	case16
//

static CaseRange16 case16[] = {
	{0x0041, 0x005a,      0,     32}, {0x0061, 0x007a,    -32,      0}, {0x00b5, 0x00b5,    743,      0},
	{0x00c0, 0x00d6,      0,     32}, {0x00d8, 0x00de,      0,     32}, {0x00e0, 0x00f6,    -32,      0},
	{0x00f8, 0x00fe,    -32,      0}, {0x00ff, 0x00ff,    121,      0}, {0x0100, 0x0100,      0,      1},
	{0x0101, 0x0101,     -1,      0}, {0x0102, 0x0102,      0,      1}, {0x0103, 0x0103,     -1,      0},
	{0x0104, 0x0104,      0,      1}, {0x0105, 0x0105,     -1,      0}, {0x0106, 0x0106,      0,      1},
	{0x0107, 0x0107,     -1,      0}, {0x0108, 0x0108,      0,      1}, {0x0109, 0x0109,     -1,      0},
	{0x010a, 0x010a,      0,      1}, {0x010b, 0x010b,     -1,      0}, {0x010c, 0x010c,      0,      1},
	{0x010d, 0x010d,     -1,      0}, {0x010e, 0x010e,      0,      1}, {0x010f, 0x010f,     -1,      0},
	{0x0110, 0x0110,      0,      1}, {0x0111, 0x0111,     -1,      0}, {0x0112, 0x0112,      0,      1},
	{0x0113, 0x0113,     -1,      0}, {0x0114, 0x0114,      0,      1}, {0x0115, 0x0115,     -1,      0},
	{0x0116, 0x0116,      0,      1}, {0x0117, 0x0117,     -1,      0}, {0x0118, 0x0118,      0,      1},
	{0x0119, 0x0119,     -1,      0}, {0x011a, 0x011a,      0,      1}, {0x011b, 0x011b,     -1,      0},
	{0x011c, 0x011c,      0,      1}, {0x011d, 0x011d,     -1,      0}, {0x011e, 0x011e,      0,      1},
	{0x011f, 0x011f,     -1,      0}, {0x0120, 0x0120,      0,      1}, {0x0121, 0x0121,     -1,      0},
	{0x0122, 0x0122,      0,      1}, {0x0123, 0x0123,     -1,      0}, {0x0124, 0x0124,      0,      1},
	{0x0125, 0x0125,     -1,      0}, {0x0126, 0x0126,      0,      1}, {0x0127, 0x0127,     -1,      0},
	{0x0128, 0x0128,      0,      1}, {0x0129, 0x0129,     -1,      0}, {0x012a, 0x012a,      0,      1},
	{0x012b, 0x012b,     -1,      0}, {0x012c, 0x012c,      0,      1}, {0x012d, 0x012d,     -1,      0},
	{0x012e, 0x012e,      0,      1}, {0x012f, 0x012f,     -1,      0}, {0x0130, 0x0130,      0,   -199},
	{0x0131, 0x0131,   -232,      0}, {0x0132, 0x0132,      0,      1}, {0x0133, 0x0133,     -1,      0},
	{0x0134, 0x0134,      0,      1}, {0x0135, 0x0135,     -1,      0}, {0x0136, 0x0136,      0,      1},
	{0x0137, 0x0137,     -1,      0}, {0x0139, 0x0139,      0,      1}, {0x013a, 0x013a,     -1,      0},
	{0x013b, 0x013b,      0,      1}, {0x013c, 0x013c,     -1,      0}, {0x013d, 0x013d,      0,      1},
	{0x013e, 0x013e,     -1,      0}, {0x013f, 0x013f,      0,      1}, {0x0140, 0x0140,     -1,      0},
	{0x0141, 0x0141,      0,      1}, {0x0142, 0x0142,     -1,      0}, {0x0143, 0x0143,      0,      1},
	{0x0144, 0x0144,     -1,      0}, {0x0145, 0x0145,      0,      1}, {0x0146, 0x0146,     -1,      0},
	{0x0147, 0x0147,      0,      1}, {0x0148, 0x0148,     -1,      0}, {0x014a, 0x014a,      0,      1},
	{0x014b, 0x014b,     -1,      0}, {0x014c, 0x014c,      0,      1}, {0x014d, 0x014d,     -1,      0},
	{0x014e, 0x014e,      0,      1}, {0x014f, 0x014f,     -1,      0}, {0x0150, 0x0150,      0,      1},
	{0x0151, 0x0151,     -1,      0}, {0x0152, 0x0152,      0,      1}, {0x0153, 0x0153,     -1,      0},
	{0x0154, 0x0154,      0,      1}, {0x0155, 0x0155,     -1,      0}, {0x0156, 0x0156,      0,      1},
	{0x0157, 0x0157,     -1,      0}, {0x0158, 0x0158,      0,      1}, {0x0159, 0x0159,     -1,      0},
	{0x015a, 0x015a,      0,      1}, {0x015b, 0x015b,     -1,      0}, {0x015c, 0x015c,      0,      1},
	{0x015d, 0x015d,     -1,      0}, {0x015e, 0x015e,      0,      1}, {0x015f, 0x015f,     -1,      0},
	{0x0160, 0x0160,      0,      1}, {0x0161, 0x0161,     -1,      0}, {0x0162, 0x0162,      0,      1},
	{0x0163, 0x0163,     -1,      0}, {0x0164, 0x0164,      0,      1}, {0x0165, 0x0165,     -1,      0},
	{0x0166, 0x0166,      0,      1}, {0x0167, 0x0167,     -1,      0}, {0x0168, 0x0168,      0,      1},
	{0x0169, 0x0169,     -1,      0}, {0x016a, 0x016a,      0,      1}, {0x016b, 0x016b,     -1,      0},
	{0x016c, 0x016c,      0,      1}, {0x016d, 0x016d,     -1,      0}, {0x016e, 0x016e,      0,      1},
	{0x016f, 0x016f,     -1,      0}, {0x0170, 0x0170,      0,      1}, {0x0171, 0x0171,     -1,      0},
	{0x0172, 0x0172,      0,      1}, {0x0173, 0x0173,     -1,      0}, {0x0174, 0x0174,      0,      1},
	{0x0175, 0x0175,     -1,      0}, {0x0176, 0x0176,      0,      1}, {0x0177, 0x0177,     -1,      0},
	{0x0178, 0x0178,      0,   -121}, {0x0179, 0x0179,      0,      1}, {0x017a, 0x017a,     -1,      0},
	{0x017b, 0x017b,      0,      1}, {0x017c, 0x017c,     -1,      0}, {0x017d, 0x017d,      0,      1},
	{0x017e, 0x017e,     -1,      0}, {0x017f, 0x017f,   -300,      0}, {0x0180, 0x0180,    195,      0},
	{0x0181, 0x0181,      0,    210}, {0x0182, 0x0182,      0,      1}, {0x0183, 0x0183,     -1,      0},
	{0x0184, 0x0184,      0,      1}, {0x0185, 0x0185,     -1,      0}, {0x0186, 0x0186,      0,    206},
	{0x0187, 0x0187,      0,      1}, {0x0188, 0x0188,     -1,      0}, {0x0189, 0x018a,      0,    205},
	{0x018b, 0x018b,      0,      1}, {0x018c, 0x018c,     -1,      0}, {0x018e, 0x018e,      0,     79},
	{0x018f, 0x018f,      0,    202}, {0x0190, 0x0190,      0,    203}, {0x0191, 0x0191,      0,      1},
	{0x0192, 0x0192,     -1,      0}, {0x0193, 0x0193,      0,    205}, {0x0194, 0x0194,      0,    207},
	{0x0195, 0x0195,     97,      0}, {0x0196, 0x0196,      0,    211}, {0x0197, 0x0197,      0,    209},
	{0x0198, 0x0198,      0,      1}, {0x0199, 0x0199,     -1,      0}, {0x019a, 0x019a,    163,      0},
	{0x019b, 0x019b,  42561,      0}, {0x019c, 0x019c,      0,    211}, {0x019d, 0x019d,      0,    213},
	{0x019e, 0x019e,    130,      0}, {0x019f, 0x019f,      0,    214}, {0x01a0, 0x01a0,      0,      1},
	{0x01a1, 0x01a1,     -1,      0}, {0x01a2, 0x01a2,      0,      1}, {0x01a3, 0x01a3,     -1,      0},
	{0x01a4, 0x01a4,      0,      1}, {0x01a5, 0x01a5,     -1,      0}, {0x01a6, 0x01a6,      0,    218},
	{0x01a7, 0x01a7,      0,      1}, {0x01a8, 0x01a8,     -1,      0}, {0x01a9, 0x01a9,      0,    218},
	{0x01ac, 0x01ac,      0,      1}, {0x01ad, 0x01ad,     -1,      0}, {0x01ae, 0x01ae,      0,    218},
	{0x01af, 0x01af,      0,      1}, {0x01b0, 0x01b0,     -1,      0}, {0x01b1, 0x01b2,      0,    217},
	{0x01b3, 0x01b3,      0,      1}, {0x01b4, 0x01b4,     -1,      0}, {0x01b5, 0x01b5,      0,      1},
	{0x01b6, 0x01b6,     -1,      0}, {0x01b7, 0x01b7,      0,    219}, {0x01b8, 0x01b8,      0,      1},
	{0x01b9, 0x01b9,     -1,      0}, {0x01bc, 0x01bc,      0,      1}, {0x01bd, 0x01bd,     -1,      0},
	{0x01bf, 0x01bf,     56,      0}, {0x01c4, 0x01c4,      0,      2}, {0x01c6, 0x01c6,     -2,      0},
	{0x01c7, 0x01c7,      0,      2}, {0x01c9, 0x01c9,     -2,      0}, {0x01ca, 0x01ca,      0,      2},
	{0x01cc, 0x01cc,     -2,      0}, {0x01cd, 0x01cd,      0,      1}, {0x01ce, 0x01ce,     -1,      0},
	{0x01cf, 0x01cf,      0,      1}, {0x01d0, 0x01d0,     -1,      0}, {0x01d1, 0x01d1,      0,      1},
	{0x01d2, 0x01d2,     -1,      0}, {0x01d3, 0x01d3,      0,      1}, {0x01d4, 0x01d4,     -1,      0},
	{0x01d5, 0x01d5,      0,      1}, {0x01d6, 0x01d6,     -1,      0}, {0x01d7, 0x01d7,      0,      1},
	{0x01d8, 0x01d8,     -1,      0}, {0x01d9, 0x01d9,      0,      1}, {0x01da, 0x01da,     -1,      0},
	{0x01db, 0x01db,      0,      1}, {0x01dc, 0x01dc,     -1,      0}, {0x01dd, 0x01dd,    -79,      0},
	{0x01de, 0x01de,      0,      1}, {0x01df, 0x01df,     -1,      0}, {0x01e0, 0x01e0,      0,      1},
	{0x01e1, 0x01e1,     -1,      0}, {0x01e2, 0x01e2,      0,      1}, {0x01e3, 0x01e3,     -1,      0},
	{0x01e4, 0x01e4,      0,      1}, {0x01e5, 0x01e5,     -1,      0}, {0x01e6, 0x01e6,      0,      1},
	{0x01e7, 0x01e7,     -1,      0}, {0x01e8, 0x01e8,      0,      1}, {0x01e9, 0x01e9,     -1,      0},
	{0x01ea, 0x01ea,      0,      1}, {0x01eb, 0x01eb,     -1,      0}, {0x01ec, 0x01ec,      0,      1},
	{0x01ed, 0x01ed,     -1,      0}, {0x01ee, 0x01ee,      0,      1}, {0x01ef, 0x01ef,     -1,      0},
	{0x01f1, 0x01f1,      0,      2}, {0x01f3, 0x01f3,     -2,      0}, {0x01f4, 0x01f4,      0,      1},
	{0x01f5, 0x01f5,     -1,      0}, {0x01f6, 0x01f6,      0,    -97}, {0x01f7, 0x01f7,      0,    -56},
	{0x01f8, 0x01f8,      0,      1}, {0x01f9, 0x01f9,     -1,      0}, {0x01fa, 0x01fa,      0,      1},
	{0x01fb, 0x01fb,     -1,      0}, {0x01fc, 0x01fc,      0,      1}, {0x01fd, 0x01fd,     -1,      0},
	{0x01fe, 0x01fe,      0,      1}, {0x01ff, 0x01ff,     -1,      0}, {0x0200, 0x0200,      0,      1},
	{0x0201, 0x0201,     -1,      0}, {0x0202, 0x0202,      0,      1}, {0x0203, 0x0203,     -1,      0},
	{0x0204, 0x0204,      0,      1}, {0x0205, 0x0205,     -1,      0}, {0x0206, 0x0206,      0,      1},
	{0x0207, 0x0207,     -1,      0}, {0x0208, 0x0208,      0,      1}, {0x0209, 0x0209,     -1,      0},
	{0x020a, 0x020a,      0,      1}, {0x020b, 0x020b,     -1,      0}, {0x020c, 0x020c,      0,      1},
	{0x020d, 0x020d,     -1,      0}, {0x020e, 0x020e,      0,      1}, {0x020f, 0x020f,     -1,      0},
	{0x0210, 0x0210,      0,      1}, {0x0211, 0x0211,     -1,      0}, {0x0212, 0x0212,      0,      1},
	{0x0213, 0x0213,     -1,      0}, {0x0214, 0x0214,      0,      1}, {0x0215, 0x0215,     -1,      0},
	{0x0216, 0x0216,      0,      1}, {0x0217, 0x0217,     -1,      0}, {0x0218, 0x0218,      0,      1},
	{0x0219, 0x0219,     -1,      0}, {0x021a, 0x021a,      0,      1}, {0x021b, 0x021b,     -1,      0},
	{0x021c, 0x021c,      0,      1}, {0x021d, 0x021d,     -1,      0}, {0x021e, 0x021e,      0,      1},
	{0x021f, 0x021f,     -1,      0}, {0x0220, 0x0220,      0,   -130}, {0x0222, 0x0222,      0,      1},
	{0x0223, 0x0223,     -1,      0}, {0x0224, 0x0224,      0,      1}, {0x0225, 0x0225,     -1,      0},
	{0x0226, 0x0226,      0,      1}, {0x0227, 0x0227,     -1,      0}, {0x0228, 0x0228,      0,      1},
	{0x0229, 0x0229,     -1,      0}, {0x022a, 0x022a,      0,      1}, {0x022b, 0x022b,     -1,      0},
	{0x022c, 0x022c,      0,      1}, {0x022d, 0x022d,     -1,      0}, {0x022e, 0x022e,      0,      1},
	{0x022f, 0x022f,     -1,      0}, {0x0230, 0x0230,      0,      1}, {0x0231, 0x0231,     -1,      0},
	{0x0232, 0x0232,      0,      1}, {0x0233, 0x0233,     -1,      0}, {0x023a, 0x023a,      0,  10795},
	{0x023b, 0x023b,      0,      1}, {0x023c, 0x023c,     -1,      0}, {0x023d, 0x023d,      0,   -163},
	{0x023e, 0x023e,      0,  10792}, {0x023f, 0x0240,  10815,      0}, {0x0241, 0x0241,      0,      1},
	{0x0242, 0x0242,     -1,      0}, {0x0243, 0x0243,      0,   -195}, {0x0244, 0x0244,      0,     69},
	{0x0245, 0x0245,      0,     71}, {0x0246, 0x0246,      0,      1}, {0x0247, 0x0247,     -1,      0},
	{0x0248, 0x0248,      0,      1}, {0x0249, 0x0249,     -1,      0}, {0x024a, 0x024a,      0,      1},
	{0x024b, 0x024b,     -1,      0}, {0x024c, 0x024c,      0,      1}, {0x024d, 0x024d,     -1,      0},
	{0x024e, 0x024e,      0,      1}, {0x024f, 0x024f,     -1,      0}, {0x0250, 0x0250,  10783,      0},
	{0x0251, 0x0251,  10780,      0}, {0x0252, 0x0252,  10782,      0}, {0x0253, 0x0253,   -210,      0},
	{0x0254, 0x0254,   -206,      0}, {0x0256, 0x0257,   -205,      0}, {0x0259, 0x0259,   -202,      0},
	{0x025b, 0x025b,   -203,      0}, {0x025c, 0x025c,  42319,      0}, {0x0260, 0x0260,   -205,      0},
	{0x0261, 0x0261,  42315,      0}, {0x0263, 0x0263,   -207,      0}, {0x0264, 0x0264,  42343,      0},
	{0x0265, 0x0265,  42280,      0}, {0x0266, 0x0266,  42308,      0}, {0x0268, 0x0268,   -209,      0},
	{0x0269, 0x0269,   -211,      0}, {0x026a, 0x026a,  42308,      0}, {0x026b, 0x026b,  10743,      0},
	{0x026c, 0x026c,  42305,      0}, {0x026f, 0x026f,   -211,      0}, {0x0271, 0x0271,  10749,      0},
	{0x0272, 0x0272,   -213,      0}, {0x0275, 0x0275,   -214,      0}, {0x0277, 0x0277,  42342,      0},
	{0x027c, 0x027c,  42342,      0}, {0x027d, 0x027d,  10727,      0}, {0x0280, 0x0280,   -218,      0},
	{0x0282, 0x0282,  42307,      0}, {0x0283, 0x0283,   -218,      0}, {0x0287, 0x0287,  42282,      0},
	{0x0288, 0x0288,   -218,      0}, {0x0289, 0x0289,    -69,      0}, {0x028a, 0x028b,   -217,      0},
	{0x028c, 0x028c,    -71,      0}, {0x0292, 0x0292,   -219,      0}, {0x029d, 0x029d,  42261,      0},
	{0x029e, 0x029e,  42258,      0}, {0x0370, 0x0370,      0,      1}, {0x0371, 0x0371,     -1,      0},
	{0x0372, 0x0372,      0,      1}, {0x0373, 0x0373,     -1,      0}, {0x0376, 0x0376,      0,      1},
	{0x0377, 0x0377,     -1,      0}, {0x037b, 0x037d,    130,      0}, {0x037f, 0x037f,      0,    116},
	{0x0386, 0x0386,      0,     38}, {0x0388, 0x038a,      0,     37}, {0x038c, 0x038c,      0,     64},
	{0x038e, 0x038f,      0,     63}, {0x0391, 0x03a1,      0,     32}, {0x03a3, 0x03ab,      0,     32},
	{0x03ac, 0x03ac,    -38,      0}, {0x03ad, 0x03af,    -37,      0}, {0x03b1, 0x03c1,    -32,      0},
	{0x03c2, 0x03c2,    -31,      0}, {0x03c3, 0x03cb,    -32,      0}, {0x03cc, 0x03cc,    -64,      0},
	{0x03cd, 0x03ce,    -63,      0}, {0x03cf, 0x03cf,      0,      8}, {0x03d0, 0x03d0,    -62,      0},
	{0x03d1, 0x03d1,    -57,      0}, {0x03d5, 0x03d5,    -47,      0}, {0x03d6, 0x03d6,    -54,      0},
	{0x03d7, 0x03d7,     -8,      0}, {0x03d8, 0x03d8,      0,      1}, {0x03d9, 0x03d9,     -1,      0},
	{0x03da, 0x03da,      0,      1}, {0x03db, 0x03db,     -1,      0}, {0x03dc, 0x03dc,      0,      1},
	{0x03dd, 0x03dd,     -1,      0}, {0x03de, 0x03de,      0,      1}, {0x03df, 0x03df,     -1,      0},
	{0x03e0, 0x03e0,      0,      1}, {0x03e1, 0x03e1,     -1,      0}, {0x03e2, 0x03e2,      0,      1},
	{0x03e3, 0x03e3,     -1,      0}, {0x03e4, 0x03e4,      0,      1}, {0x03e5, 0x03e5,     -1,      0},
	{0x03e6, 0x03e6,      0,      1}, {0x03e7, 0x03e7,     -1,      0}, {0x03e8, 0x03e8,      0,      1},
	{0x03e9, 0x03e9,     -1,      0}, {0x03ea, 0x03ea,      0,      1}, {0x03eb, 0x03eb,     -1,      0},
	{0x03ec, 0x03ec,      0,      1}, {0x03ed, 0x03ed,     -1,      0}, {0x03ee, 0x03ee,      0,      1},
	{0x03ef, 0x03ef,     -1,      0}, {0x03f0, 0x03f0,    -86,      0}, {0x03f1, 0x03f1,    -80,      0},
	{0x03f2, 0x03f2,      7,      0}, {0x03f3, 0x03f3,   -116,      0}, {0x03f4, 0x03f4,      0,    -60},
	{0x03f5, 0x03f5,    -96,      0}, {0x03f7, 0x03f7,      0,      1}, {0x03f8, 0x03f8,     -1,      0},
	{0x03f9, 0x03f9,      0,     -7}, {0x03fa, 0x03fa,      0,      1}, {0x03fb, 0x03fb,     -1,      0},
	{0x03fd, 0x03ff,      0,   -130}, {0x0400, 0x040f,      0,     80}, {0x0410, 0x042f,      0,     32},
	{0x0430, 0x044f,    -32,      0}, {0x0450, 0x045f,    -80,      0}, {0x0460, 0x0460,      0,      1},
	{0x0461, 0x0461,     -1,      0}, {0x0462, 0x0462,      0,      1}, {0x0463, 0x0463,     -1,      0},
	{0x0464, 0x0464,      0,      1}, {0x0465, 0x0465,     -1,      0}, {0x0466, 0x0466,      0,      1},
	{0x0467, 0x0467,     -1,      0}, {0x0468, 0x0468,      0,      1}, {0x0469, 0x0469,     -1,      0},
	{0x046a, 0x046a,      0,      1}, {0x046b, 0x046b,     -1,      0}, {0x046c, 0x046c,      0,      1},
	{0x046d, 0x046d,     -1,      0}, {0x046e, 0x046e,      0,      1}, {0x046f, 0x046f,     -1,      0},
	{0x0470, 0x0470,      0,      1}, {0x0471, 0x0471,     -1,      0}, {0x0472, 0x0472,      0,      1},
	{0x0473, 0x0473,     -1,      0}, {0x0474, 0x0474,      0,      1}, {0x0475, 0x0475,     -1,      0},
	{0x0476, 0x0476,      0,      1}, {0x0477, 0x0477,     -1,      0}, {0x0478, 0x0478,      0,      1},
	{0x0479, 0x0479,     -1,      0}, {0x047a, 0x047a,      0,      1}, {0x047b, 0x047b,     -1,      0},
	{0x047c, 0x047c,      0,      1}, {0x047d, 0x047d,     -1,      0}, {0x047e, 0x047e,      0,      1},
	{0x047f, 0x047f,     -1,      0}, {0x0480, 0x0480,      0,      1}, {0x0481, 0x0481,     -1,      0},
	{0x048a, 0x048a,      0,      1}, {0x048b, 0x048b,     -1,      0}, {0x048c, 0x048c,      0,      1},
	{0x048d, 0x048d,     -1,      0}, {0x048e, 0x048e,      0,      1}, {0x048f, 0x048f,     -1,      0},
	{0x0490, 0x0490,      0,      1}, {0x0491, 0x0491,     -1,      0}, {0x0492, 0x0492,      0,      1},
	{0x0493, 0x0493,     -1,      0}, {0x0494, 0x0494,      0,      1}, {0x0495, 0x0495,     -1,      0},
	{0x0496, 0x0496,      0,      1}, {0x0497, 0x0497,     -1,      0}, {0x0498, 0x0498,      0,      1},
	{0x0499, 0x0499,     -1,      0}, {0x049a, 0x049a,      0,      1}, {0x049b, 0x049b,     -1,      0},
	{0x049c, 0x049c,      0,      1}, {0x049d, 0x049d,     -1,      0}, {0x049e, 0x049e,      0,      1},
	{0x049f, 0x049f,     -1,      0}, {0x04a0, 0x04a0,      0,      1}, {0x04a1, 0x04a1,     -1,      0},
	{0x04a2, 0x04a2,      0,      1}, {0x04a3, 0x04a3,     -1,      0}, {0x04a4, 0x04a4,      0,      1},
	{0x04a5, 0x04a5,     -1,      0}, {0x04a6, 0x04a6,      0,      1}, {0x04a7, 0x04a7,     -1,      0},
	{0x04a8, 0x04a8,      0,      1}, {0x04a9, 0x04a9,     -1,      0}, {0x04aa, 0x04aa,      0,      1},
	{0x04ab, 0x04ab,     -1,      0}, {0x04ac, 0x04ac,      0,      1}, {0x04ad, 0x04ad,     -1,      0},
	{0x04ae, 0x04ae,      0,      1}, {0x04af, 0x04af,     -1,      0}, {0x04b0, 0x04b0,      0,      1},
	{0x04b1, 0x04b1,     -1,      0}, {0x04b2, 0x04b2,      0,      1}, {0x04b3, 0x04b3,     -1,      0},
	{0x04b4, 0x04b4,      0,      1}, {0x04b5, 0x04b5,     -1,      0}, {0x04b6, 0x04b6,      0,      1},
	{0x04b7, 0x04b7,     -1,      0}, {0x04b8, 0x04b8,      0,      1}, {0x04b9, 0x04b9,     -1,      0},
	{0x04ba, 0x04ba,      0,      1}, {0x04bb, 0x04bb,     -1,      0}, {0x04bc, 0x04bc,      0,      1},
	{0x04bd, 0x04bd,     -1,      0}, {0x04be, 0x04be,      0,      1}, {0x04bf, 0x04bf,     -1,      0},
	{0x04c0, 0x04c0,      0,     15}, {0x04c1, 0x04c1,      0,      1}, {0x04c2, 0x04c2,     -1,      0},
	{0x04c3, 0x04c3,      0,      1}, {0x04c4, 0x04c4,     -1,      0}, {0x04c5, 0x04c5,      0,      1},
	{0x04c6, 0x04c6,     -1,      0}, {0x04c7, 0x04c7,      0,      1}, {0x04c8, 0x04c8,     -1,      0},
	{0x04c9, 0x04c9,      0,      1}, {0x04ca, 0x04ca,     -1,      0}, {0x04cb, 0x04cb,      0,      1},
	{0x04cc, 0x04cc,     -1,      0}, {0x04cd, 0x04cd,      0,      1}, {0x04ce, 0x04ce,     -1,      0},
	{0x04cf, 0x04cf,    -15,      0}, {0x04d0, 0x04d0,      0,      1}, {0x04d1, 0x04d1,     -1,      0},
	{0x04d2, 0x04d2,      0,      1}, {0x04d3, 0x04d3,     -1,      0}, {0x04d4, 0x04d4,      0,      1},
	{0x04d5, 0x04d5,     -1,      0}, {0x04d6, 0x04d6,      0,      1}, {0x04d7, 0x04d7,     -1,      0},
	{0x04d8, 0x04d8,      0,      1}, {0x04d9, 0x04d9,     -1,      0}, {0x04da, 0x04da,      0,      1},
	{0x04db, 0x04db,     -1,      0}, {0x04dc, 0x04dc,      0,      1}, {0x04dd, 0x04dd,     -1,      0},
	{0x04de, 0x04de,      0,      1}, {0x04df, 0x04df,     -1,      0}, {0x04e0, 0x04e0,      0,      1},
	{0x04e1, 0x04e1,     -1,      0}, {0x04e2, 0x04e2,      0,      1}, {0x04e3, 0x04e3,     -1,      0},
	{0x04e4, 0x04e4,      0,      1}, {0x04e5, 0x04e5,     -1,      0}, {0x04e6, 0x04e6,      0,      1},
	{0x04e7, 0x04e7,     -1,      0}, {0x04e8, 0x04e8,      0,      1}, {0x04e9, 0x04e9,     -1,      0},
	{0x04ea, 0x04ea,      0,      1}, {0x04eb, 0x04eb,     -1,      0}, {0x04ec, 0x04ec,      0,      1},
	{0x04ed, 0x04ed,     -1,      0}, {0x04ee, 0x04ee,      0,      1}, {0x04ef, 0x04ef,     -1,      0},
	{0x04f0, 0x04f0,      0,      1}, {0x04f1, 0x04f1,     -1,      0}, {0x04f2, 0x04f2,      0,      1},
	{0x04f3, 0x04f3,     -1,      0}, {0x04f4, 0x04f4,      0,      1}, {0x04f5, 0x04f5,     -1,      0},
	{0x04f6, 0x04f6,      0,      1}, {0x04f7, 0x04f7,     -1,      0}, {0x04f8, 0x04f8,      0,      1},
	{0x04f9, 0x04f9,     -1,      0}, {0x04fa, 0x04fa,      0,      1}, {0x04fb, 0x04fb,     -1,      0},
	{0x04fc, 0x04fc,      0,      1}, {0x04fd, 0x04fd,     -1,      0}, {0x04fe, 0x04fe,      0,      1},
	{0x04ff, 0x04ff,     -1,      0}, {0x0500, 0x0500,      0,      1}, {0x0501, 0x0501,     -1,      0},
	{0x0502, 0x0502,      0,      1}, {0x0503, 0x0503,     -1,      0}, {0x0504, 0x0504,      0,      1},
	{0x0505, 0x0505,     -1,      0}, {0x0506, 0x0506,      0,      1}, {0x0507, 0x0507,     -1,      0},
	{0x0508, 0x0508,      0,      1}, {0x0509, 0x0509,     -1,      0}, {0x050a, 0x050a,      0,      1},
	{0x050b, 0x050b,     -1,      0}, {0x050c, 0x050c,      0,      1}, {0x050d, 0x050d,     -1,      0},
	{0x050e, 0x050e,      0,      1}, {0x050f, 0x050f,     -1,      0}, {0x0510, 0x0510,      0,      1},
	{0x0511, 0x0511,     -1,      0}, {0x0512, 0x0512,      0,      1}, {0x0513, 0x0513,     -1,      0},
	{0x0514, 0x0514,      0,      1}, {0x0515, 0x0515,     -1,      0}, {0x0516, 0x0516,      0,      1},
	{0x0517, 0x0517,     -1,      0}, {0x0518, 0x0518,      0,      1}, {0x0519, 0x0519,     -1,      0},
	{0x051a, 0x051a,      0,      1}, {0x051b, 0x051b,     -1,      0}, {0x051c, 0x051c,      0,      1},
	{0x051d, 0x051d,     -1,      0}, {0x051e, 0x051e,      0,      1}, {0x051f, 0x051f,     -1,      0},
	{0x0520, 0x0520,      0,      1}, {0x0521, 0x0521,     -1,      0}, {0x0522, 0x0522,      0,      1},
	{0x0523, 0x0523,     -1,      0}, {0x0524, 0x0524,      0,      1}, {0x0525, 0x0525,     -1,      0},
	{0x0526, 0x0526,      0,      1}, {0x0527, 0x0527,     -1,      0}, {0x0528, 0x0528,      0,      1},
	{0x0529, 0x0529,     -1,      0}, {0x052a, 0x052a,      0,      1}, {0x052b, 0x052b,     -1,      0},
	{0x052c, 0x052c,      0,      1}, {0x052d, 0x052d,     -1,      0}, {0x052e, 0x052e,      0,      1},
	{0x052f, 0x052f,     -1,      0}, {0x0531, 0x0556,      0,     48}, {0x0561, 0x0586,    -48,      0},
	{0x10a0, 0x10c5,      0,   7264}, {0x10c7, 0x10c7,      0,   7264}, {0x10cd, 0x10cd,      0,   7264},
	{0x10d0, 0x10fa,   3008,      0}, {0x10fd, 0x10ff,   3008,      0}, {0x13a0, 0x13ef,      0,  38864},
	{0x13f0, 0x13f5,      0,      8}, {0x13f8, 0x13fd,     -8,      0}, {0x1c80, 0x1c80,  -6254,      0},
	{0x1c81, 0x1c81,  -6253,      0}, {0x1c82, 0x1c82,  -6244,      0}, {0x1c83, 0x1c84,  -6242,      0},
	{0x1c85, 0x1c85,  -6243,      0}, {0x1c86, 0x1c86,  -6236,      0}, {0x1c87, 0x1c87,  -6181,      0},
	{0x1c88, 0x1c88,  35266,      0}, {0x1c89, 0x1c89,      0,      1}, {0x1c8a, 0x1c8a,     -1,      0},
	{0x1c90, 0x1cba,      0,  -3008}, {0x1cbd, 0x1cbf,      0,  -3008}, {0x1d79, 0x1d79,  35332,      0},
	{0x1d7d, 0x1d7d,   3814,      0}, {0x1d8e, 0x1d8e,  35384,      0}, {0x1e00, 0x1e00,      0,      1},
	{0x1e01, 0x1e01,     -1,      0}, {0x1e02, 0x1e02,      0,      1}, {0x1e03, 0x1e03,     -1,      0},
	{0x1e04, 0x1e04,      0,      1}, {0x1e05, 0x1e05,     -1,      0}, {0x1e06, 0x1e06,      0,      1},
	{0x1e07, 0x1e07,     -1,      0}, {0x1e08, 0x1e08,      0,      1}, {0x1e09, 0x1e09,     -1,      0},
	{0x1e0a, 0x1e0a,      0,      1}, {0x1e0b, 0x1e0b,     -1,      0}, {0x1e0c, 0x1e0c,      0,      1},
	{0x1e0d, 0x1e0d,     -1,      0}, {0x1e0e, 0x1e0e,      0,      1}, {0x1e0f, 0x1e0f,     -1,      0},
	{0x1e10, 0x1e10,      0,      1}, {0x1e11, 0x1e11,     -1,      0}, {0x1e12, 0x1e12,      0,      1},
	{0x1e13, 0x1e13,     -1,      0}, {0x1e14, 0x1e14,      0,      1}, {0x1e15, 0x1e15,     -1,      0},
	{0x1e16, 0x1e16,      0,      1}, {0x1e17, 0x1e17,     -1,      0}, {0x1e18, 0x1e18,      0,      1},
	{0x1e19, 0x1e19,     -1,      0}, {0x1e1a, 0x1e1a,      0,      1}, {0x1e1b, 0x1e1b,     -1,      0},
	{0x1e1c, 0x1e1c,      0,      1}, {0x1e1d, 0x1e1d,     -1,      0}, {0x1e1e, 0x1e1e,      0,      1},
	{0x1e1f, 0x1e1f,     -1,      0}, {0x1e20, 0x1e20,      0,      1}, {0x1e21, 0x1e21,     -1,      0},
	{0x1e22, 0x1e22,      0,      1}, {0x1e23, 0x1e23,     -1,      0}, {0x1e24, 0x1e24,      0,      1},
	{0x1e25, 0x1e25,     -1,      0}, {0x1e26, 0x1e26,      0,      1}, {0x1e27, 0x1e27,     -1,      0},
	{0x1e28, 0x1e28,      0,      1}, {0x1e29, 0x1e29,     -1,      0}, {0x1e2a, 0x1e2a,      0,      1},
	{0x1e2b, 0x1e2b,     -1,      0}, {0x1e2c, 0x1e2c,      0,      1}, {0x1e2d, 0x1e2d,     -1,      0},
	{0x1e2e, 0x1e2e,      0,      1}, {0x1e2f, 0x1e2f,     -1,      0}, {0x1e30, 0x1e30,      0,      1},
	{0x1e31, 0x1e31,     -1,      0}, {0x1e32, 0x1e32,      0,      1}, {0x1e33, 0x1e33,     -1,      0},
	{0x1e34, 0x1e34,      0,      1}, {0x1e35, 0x1e35,     -1,      0}, {0x1e36, 0x1e36,      0,      1},
	{0x1e37, 0x1e37,     -1,      0}, {0x1e38, 0x1e38,      0,      1}, {0x1e39, 0x1e39,     -1,      0},
	{0x1e3a, 0x1e3a,      0,      1}, {0x1e3b, 0x1e3b,     -1,      0}, {0x1e3c, 0x1e3c,      0,      1},
	{0x1e3d, 0x1e3d,     -1,      0}, {0x1e3e, 0x1e3e,      0,      1}, {0x1e3f, 0x1e3f,     -1,      0},
	{0x1e40, 0x1e40,      0,      1}, {0x1e41, 0x1e41,     -1,      0}, {0x1e42, 0x1e42,      0,      1},
	{0x1e43, 0x1e43,     -1,      0}, {0x1e44, 0x1e44,      0,      1}, {0x1e45, 0x1e45,     -1,      0},
	{0x1e46, 0x1e46,      0,      1}, {0x1e47, 0x1e47,     -1,      0}, {0x1e48, 0x1e48,      0,      1},
	{0x1e49, 0x1e49,     -1,      0}, {0x1e4a, 0x1e4a,      0,      1}, {0x1e4b, 0x1e4b,     -1,      0},
	{0x1e4c, 0x1e4c,      0,      1}, {0x1e4d, 0x1e4d,     -1,      0}, {0x1e4e, 0x1e4e,      0,      1},
	{0x1e4f, 0x1e4f,     -1,      0}, {0x1e50, 0x1e50,      0,      1}, {0x1e51, 0x1e51,     -1,      0},
	{0x1e52, 0x1e52,      0,      1}, {0x1e53, 0x1e53,     -1,      0}, {0x1e54, 0x1e54,      0,      1},
	{0x1e55, 0x1e55,     -1,      0}, {0x1e56, 0x1e56,      0,      1}, {0x1e57, 0x1e57,     -1,      0},
	{0x1e58, 0x1e58,      0,      1}, {0x1e59, 0x1e59,     -1,      0}, {0x1e5a, 0x1e5a,      0,      1},
	{0x1e5b, 0x1e5b,     -1,      0}, {0x1e5c, 0x1e5c,      0,      1}, {0x1e5d, 0x1e5d,     -1,      0},
	{0x1e5e, 0x1e5e,      0,      1}, {0x1e5f, 0x1e5f,     -1,      0}, {0x1e60, 0x1e60,      0,      1},
	{0x1e61, 0x1e61,     -1,      0}, {0x1e62, 0x1e62,      0,      1}, {0x1e63, 0x1e63,     -1,      0},
	{0x1e64, 0x1e64,      0,      1}, {0x1e65, 0x1e65,     -1,      0}, {0x1e66, 0x1e66,      0,      1},
	{0x1e67, 0x1e67,     -1,      0}, {0x1e68, 0x1e68,      0,      1}, {0x1e69, 0x1e69,     -1,      0},
	{0x1e6a, 0x1e6a,      0,      1}, {0x1e6b, 0x1e6b,     -1,      0}, {0x1e6c, 0x1e6c,      0,      1},
	{0x1e6d, 0x1e6d,     -1,      0}, {0x1e6e, 0x1e6e,      0,      1}, {0x1e6f, 0x1e6f,     -1,      0},
	{0x1e70, 0x1e70,      0,      1}, {0x1e71, 0x1e71,     -1,      0}, {0x1e72, 0x1e72,      0,      1},
	{0x1e73, 0x1e73,     -1,      0}, {0x1e74, 0x1e74,      0,      1}, {0x1e75, 0x1e75,     -1,      0},
	{0x1e76, 0x1e76,      0,      1}, {0x1e77, 0x1e77,     -1,      0}, {0x1e78, 0x1e78,      0,      1},
	{0x1e79, 0x1e79,     -1,      0}, {0x1e7a, 0x1e7a,      0,      1}, {0x1e7b, 0x1e7b,     -1,      0},
	{0x1e7c, 0x1e7c,      0,      1}, {0x1e7d, 0x1e7d,     -1,      0}, {0x1e7e, 0x1e7e,      0,      1},
	{0x1e7f, 0x1e7f,     -1,      0}, {0x1e80, 0x1e80,      0,      1}, {0x1e81, 0x1e81,     -1,      0},
	{0x1e82, 0x1e82,      0,      1}, {0x1e83, 0x1e83,     -1,      0}, {0x1e84, 0x1e84,      0,      1},
	{0x1e85, 0x1e85,     -1,      0}, {0x1e86, 0x1e86,      0,      1}, {0x1e87, 0x1e87,     -1,      0},
	{0x1e88, 0x1e88,      0,      1}, {0x1e89, 0x1e89,     -1,      0}, {0x1e8a, 0x1e8a,      0,      1},
	{0x1e8b, 0x1e8b,     -1,      0}, {0x1e8c, 0x1e8c,      0,      1}, {0x1e8d, 0x1e8d,     -1,      0},
	{0x1e8e, 0x1e8e,      0,      1}, {0x1e8f, 0x1e8f,     -1,      0}, {0x1e90, 0x1e90,      0,      1},
	{0x1e91, 0x1e91,     -1,      0}, {0x1e92, 0x1e92,      0,      1}, {0x1e93, 0x1e93,     -1,      0},
	{0x1e94, 0x1e94,      0,      1}, {0x1e95, 0x1e95,     -1,      0}, {0x1e9b, 0x1e9b,    -59,      0},
	{0x1e9e, 0x1e9e,      0,  -7615}, {0x1ea0, 0x1ea0,      0,      1}, {0x1ea1, 0x1ea1,     -1,      0},
	{0x1ea2, 0x1ea2,      0,      1}, {0x1ea3, 0x1ea3,     -1,      0}, {0x1ea4, 0x1ea4,      0,      1},
	{0x1ea5, 0x1ea5,     -1,      0}, {0x1ea6, 0x1ea6,      0,      1}, {0x1ea7, 0x1ea7,     -1,      0},
	{0x1ea8, 0x1ea8,      0,      1}, {0x1ea9, 0x1ea9,     -1,      0}, {0x1eaa, 0x1eaa,      0,      1},
	{0x1eab, 0x1eab,     -1,      0}, {0x1eac, 0x1eac,      0,      1}, {0x1ead, 0x1ead,     -1,      0},
	{0x1eae, 0x1eae,      0,      1}, {0x1eaf, 0x1eaf,     -1,      0}, {0x1eb0, 0x1eb0,      0,      1},
	{0x1eb1, 0x1eb1,     -1,      0}, {0x1eb2, 0x1eb2,      0,      1}, {0x1eb3, 0x1eb3,     -1,      0},
	{0x1eb4, 0x1eb4,      0,      1}, {0x1eb5, 0x1eb5,     -1,      0}, {0x1eb6, 0x1eb6,      0,      1},
	{0x1eb7, 0x1eb7,     -1,      0}, {0x1eb8, 0x1eb8,      0,      1}, {0x1eb9, 0x1eb9,     -1,      0},
	{0x1eba, 0x1eba,      0,      1}, {0x1ebb, 0x1ebb,     -1,      0}, {0x1ebc, 0x1ebc,      0,      1},
	{0x1ebd, 0x1ebd,     -1,      0}, {0x1ebe, 0x1ebe,      0,      1}, {0x1ebf, 0x1ebf,     -1,      0},
	{0x1ec0, 0x1ec0,      0,      1}, {0x1ec1, 0x1ec1,     -1,      0}, {0x1ec2, 0x1ec2,      0,      1},
	{0x1ec3, 0x1ec3,     -1,      0}, {0x1ec4, 0x1ec4,      0,      1}, {0x1ec5, 0x1ec5,     -1,      0},
	{0x1ec6, 0x1ec6,      0,      1}, {0x1ec7, 0x1ec7,     -1,      0}, {0x1ec8, 0x1ec8,      0,      1},
	{0x1ec9, 0x1ec9,     -1,      0}, {0x1eca, 0x1eca,      0,      1}, {0x1ecb, 0x1ecb,     -1,      0},
	{0x1ecc, 0x1ecc,      0,      1}, {0x1ecd, 0x1ecd,     -1,      0}, {0x1ece, 0x1ece,      0,      1},
	{0x1ecf, 0x1ecf,     -1,      0}, {0x1ed0, 0x1ed0,      0,      1}, {0x1ed1, 0x1ed1,     -1,      0},
	{0x1ed2, 0x1ed2,      0,      1}, {0x1ed3, 0x1ed3,     -1,      0}, {0x1ed4, 0x1ed4,      0,      1},
	{0x1ed5, 0x1ed5,     -1,      0}, {0x1ed6, 0x1ed6,      0,      1}, {0x1ed7, 0x1ed7,     -1,      0},
	{0x1ed8, 0x1ed8,      0,      1}, {0x1ed9, 0x1ed9,     -1,      0}, {0x1eda, 0x1eda,      0,      1},
	{0x1edb, 0x1edb,     -1,      0}, {0x1edc, 0x1edc,      0,      1}, {0x1edd, 0x1edd,     -1,      0},
	{0x1ede, 0x1ede,      0,      1}, {0x1edf, 0x1edf,     -1,      0}, {0x1ee0, 0x1ee0,      0,      1},
	{0x1ee1, 0x1ee1,     -1,      0}, {0x1ee2, 0x1ee2,      0,      1}, {0x1ee3, 0x1ee3,     -1,      0},
	{0x1ee4, 0x1ee4,      0,      1}, {0x1ee5, 0x1ee5,     -1,      0}, {0x1ee6, 0x1ee6,      0,      1},
	{0x1ee7, 0x1ee7,     -1,      0}, {0x1ee8, 0x1ee8,      0,      1}, {0x1ee9, 0x1ee9,     -1,      0},
	{0x1eea, 0x1eea,      0,      1}, {0x1eeb, 0x1eeb,     -1,      0}, {0x1eec, 0x1eec,      0,      1},
	{0x1eed, 0x1eed,     -1,      0}, {0x1eee, 0x1eee,      0,      1}, {0x1eef, 0x1eef,     -1,      0},
	{0x1ef0, 0x1ef0,      0,      1}, {0x1ef1, 0x1ef1,     -1,      0}, {0x1ef2, 0x1ef2,      0,      1},
	{0x1ef3, 0x1ef3,     -1,      0}, {0x1ef4, 0x1ef4,      0,      1}, {0x1ef5, 0x1ef5,     -1,      0},
	{0x1ef6, 0x1ef6,      0,      1}, {0x1ef7, 0x1ef7,     -1,      0}, {0x1ef8, 0x1ef8,      0,      1},
	{0x1ef9, 0x1ef9,     -1,      0}, {0x1efa, 0x1efa,      0,      1}, {0x1efb, 0x1efb,     -1,      0},
	{0x1efc, 0x1efc,      0,      1}, {0x1efd, 0x1efd,     -1,      0}, {0x1efe, 0x1efe,      0,      1},
	{0x1eff, 0x1eff,     -1,      0}, {0x1f00, 0x1f07,      8,      0}, {0x1f08, 0x1f0f,      0,     -8},
	{0x1f10, 0x1f15,      8,      0}, {0x1f18, 0x1f1d,      0,     -8}, {0x1f20, 0x1f27,      8,      0},
	{0x1f28, 0x1f2f,      0,     -8}, {0x1f30, 0x1f37,      8,      0}, {0x1f38, 0x1f3f,      0,     -8},
	{0x1f40, 0x1f45,      8,      0}, {0x1f48, 0x1f4d,      0,     -8}, {0x1f51, 0x1f51,      8,      0},
	{0x1f53, 0x1f53,      8,      0}, {0x1f55, 0x1f55,      8,      0}, {0x1f57, 0x1f57,      8,      0},
	{0x1f59, 0x1f59,      0,     -8}, {0x1f5b, 0x1f5b,      0,     -8}, {0x1f5d, 0x1f5d,      0,     -8},
	{0x1f5f, 0x1f5f,      0,     -8}, {0x1f60, 0x1f67,      8,      0}, {0x1f68, 0x1f6f,      0,     -8},
	{0x1f70, 0x1f71,     74,      0}, {0x1f72, 0x1f75,     86,      0}, {0x1f76, 0x1f77,    100,      0},
	{0x1f78, 0x1f79,    128,      0}, {0x1f7a, 0x1f7b,    112,      0}, {0x1f7c, 0x1f7d,    126,      0},
	{0x1f80, 0x1f87,      8,      0}, {0x1f90, 0x1f97,      8,      0}, {0x1fa0, 0x1fa7,      8,      0},
	{0x1fb0, 0x1fb1,      8,      0}, {0x1fb3, 0x1fb3,      9,      0}, {0x1fb8, 0x1fb9,      0,     -8},
	{0x1fba, 0x1fbb,      0,    -74}, {0x1fbe, 0x1fbe,  -7205,      0}, {0x1fc3, 0x1fc3,      9,      0},
	{0x1fc8, 0x1fcb,      0,    -86}, {0x1fd0, 0x1fd1,      8,      0}, {0x1fd8, 0x1fd9,      0,     -8},
	{0x1fda, 0x1fdb,      0,   -100}, {0x1fe0, 0x1fe1,      8,      0}, {0x1fe5, 0x1fe5,      7,      0},
	{0x1fe8, 0x1fe9,      0,     -8}, {0x1fea, 0x1feb,      0,   -112}, {0x1fec, 0x1fec,      0,     -7},
	{0x1ff3, 0x1ff3,      9,      0}, {0x1ff8, 0x1ff9,      0,   -128}, {0x1ffa, 0x1ffb,      0,   -126},
	{0x2126, 0x2126,      0,  -7517}, {0x212a, 0x212a,      0,  -8383}, {0x212b, 0x212b,      0,  -8262},
	{0x2132, 0x2132,      0,     28}, {0x214e, 0x214e,    -28,      0}, {0x2183, 0x2183,      0,      1},
	{0x2184, 0x2184,     -1,      0}, {0x2c00, 0x2c2f,      0,     48}, {0x2c30, 0x2c5f,    -48,      0},
	{0x2c60, 0x2c60,      0,      1}, {0x2c61, 0x2c61,     -1,      0}, {0x2c62, 0x2c62,      0, -10743},
	{0x2c63, 0x2c63,      0,  -3814}, {0x2c64, 0x2c64,      0, -10727}, {0x2c65, 0x2c65, -10795,      0},
	{0x2c66, 0x2c66, -10792,      0}, {0x2c67, 0x2c67,      0,      1}, {0x2c68, 0x2c68,     -1,      0},
	{0x2c69, 0x2c69,      0,      1}, {0x2c6a, 0x2c6a,     -1,      0}, {0x2c6b, 0x2c6b,      0,      1},
	{0x2c6c, 0x2c6c,     -1,      0}, {0x2c6d, 0x2c6d,      0, -10780}, {0x2c6e, 0x2c6e,      0, -10749},
	{0x2c6f, 0x2c6f,      0, -10783}, {0x2c70, 0x2c70,      0, -10782}, {0x2c72, 0x2c72,      0,      1},
	{0x2c73, 0x2c73,     -1,      0}, {0x2c75, 0x2c75,      0,      1}, {0x2c76, 0x2c76,     -1,      0},
	{0x2c7e, 0x2c7f,      0, -10815}, {0x2c80, 0x2c80,      0,      1}, {0x2c81, 0x2c81,     -1,      0},
	{0x2c82, 0x2c82,      0,      1}, {0x2c83, 0x2c83,     -1,      0}, {0x2c84, 0x2c84,      0,      1},
	{0x2c85, 0x2c85,     -1,      0}, {0x2c86, 0x2c86,      0,      1}, {0x2c87, 0x2c87,     -1,      0},
	{0x2c88, 0x2c88,      0,      1}, {0x2c89, 0x2c89,     -1,      0}, {0x2c8a, 0x2c8a,      0,      1},
	{0x2c8b, 0x2c8b,     -1,      0}, {0x2c8c, 0x2c8c,      0,      1}, {0x2c8d, 0x2c8d,     -1,      0},
	{0x2c8e, 0x2c8e,      0,      1}, {0x2c8f, 0x2c8f,     -1,      0}, {0x2c90, 0x2c90,      0,      1},
	{0x2c91, 0x2c91,     -1,      0}, {0x2c92, 0x2c92,      0,      1}, {0x2c93, 0x2c93,     -1,      0},
	{0x2c94, 0x2c94,      0,      1}, {0x2c95, 0x2c95,     -1,      0}, {0x2c96, 0x2c96,      0,      1},
	{0x2c97, 0x2c97,     -1,      0}, {0x2c98, 0x2c98,      0,      1}, {0x2c99, 0x2c99,     -1,      0},
	{0x2c9a, 0x2c9a,      0,      1}, {0x2c9b, 0x2c9b,     -1,      0}, {0x2c9c, 0x2c9c,      0,      1},
	{0x2c9d, 0x2c9d,     -1,      0}, {0x2c9e, 0x2c9e,      0,      1}, {0x2c9f, 0x2c9f,     -1,      0},
	{0x2ca0, 0x2ca0,      0,      1}, {0x2ca1, 0x2ca1,     -1,      0}, {0x2ca2, 0x2ca2,      0,      1},
	{0x2ca3, 0x2ca3,     -1,      0}, {0x2ca4, 0x2ca4,      0,      1}, {0x2ca5, 0x2ca5,     -1,      0},
	{0x2ca6, 0x2ca6,      0,      1}, {0x2ca7, 0x2ca7,     -1,      0}, {0x2ca8, 0x2ca8,      0,      1},
	{0x2ca9, 0x2ca9,     -1,      0}, {0x2caa, 0x2caa,      0,      1}, {0x2cab, 0x2cab,     -1,      0},
	{0x2cac, 0x2cac,      0,      1}, {0x2cad, 0x2cad,     -1,      0}, {0x2cae, 0x2cae,      0,      1},
	{0x2caf, 0x2caf,     -1,      0}, {0x2cb0, 0x2cb0,      0,      1}, {0x2cb1, 0x2cb1,     -1,      0},
	{0x2cb2, 0x2cb2,      0,      1}, {0x2cb3, 0x2cb3,     -1,      0}, {0x2cb4, 0x2cb4,      0,      1},
	{0x2cb5, 0x2cb5,     -1,      0}, {0x2cb6, 0x2cb6,      0,      1}, {0x2cb7, 0x2cb7,     -1,      0},
	{0x2cb8, 0x2cb8,      0,      1}, {0x2cb9, 0x2cb9,     -1,      0}, {0x2cba, 0x2cba,      0,      1},
	{0x2cbb, 0x2cbb,     -1,      0}, {0x2cbc, 0x2cbc,      0,      1}, {0x2cbd, 0x2cbd,     -1,      0},
	{0x2cbe, 0x2cbe,      0,      1}, {0x2cbf, 0x2cbf,     -1,      0}, {0x2cc0, 0x2cc0,      0,      1},
	{0x2cc1, 0x2cc1,     -1,      0}, {0x2cc2, 0x2cc2,      0,      1}, {0x2cc3, 0x2cc3,     -1,      0},
	{0x2cc4, 0x2cc4,      0,      1}, {0x2cc5, 0x2cc5,     -1,      0}, {0x2cc6, 0x2cc6,      0,      1},
	{0x2cc7, 0x2cc7,     -1,      0}, {0x2cc8, 0x2cc8,      0,      1}, {0x2cc9, 0x2cc9,     -1,      0},
	{0x2cca, 0x2cca,      0,      1}, {0x2ccb, 0x2ccb,     -1,      0}, {0x2ccc, 0x2ccc,      0,      1},
	{0x2ccd, 0x2ccd,     -1,      0}, {0x2cce, 0x2cce,      0,      1}, {0x2ccf, 0x2ccf,     -1,      0},
	{0x2cd0, 0x2cd0,      0,      1}, {0x2cd1, 0x2cd1,     -1,      0}, {0x2cd2, 0x2cd2,      0,      1},
	{0x2cd3, 0x2cd3,     -1,      0}, {0x2cd4, 0x2cd4,      0,      1}, {0x2cd5, 0x2cd5,     -1,      0},
	{0x2cd6, 0x2cd6,      0,      1}, {0x2cd7, 0x2cd7,     -1,      0}, {0x2cd8, 0x2cd8,      0,      1},
	{0x2cd9, 0x2cd9,     -1,      0}, {0x2cda, 0x2cda,      0,      1}, {0x2cdb, 0x2cdb,     -1,      0},
	{0x2cdc, 0x2cdc,      0,      1}, {0x2cdd, 0x2cdd,     -1,      0}, {0x2cde, 0x2cde,      0,      1},
	{0x2cdf, 0x2cdf,     -1,      0}, {0x2ce0, 0x2ce0,      0,      1}, {0x2ce1, 0x2ce1,     -1,      0},
	{0x2ce2, 0x2ce2,      0,      1}, {0x2ce3, 0x2ce3,     -1,      0}, {0x2ceb, 0x2ceb,      0,      1},
	{0x2cec, 0x2cec,     -1,      0}, {0x2ced, 0x2ced,      0,      1}, {0x2cee, 0x2cee,     -1,      0},
	{0x2cf2, 0x2cf2,      0,      1}, {0x2cf3, 0x2cf3,     -1,      0}, {0x2d00, 0x2d25,  -7264,      0},
	{0x2d27, 0x2d27,  -7264,      0}, {0x2d2d, 0x2d2d,  -7264,      0}, {0xa640, 0xa640,      0,      1},
	{0xa641, 0xa641,     -1,      0}, {0xa642, 0xa642,      0,      1}, {0xa643, 0xa643,     -1,      0},
	{0xa644, 0xa644,      0,      1}, {0xa645, 0xa645,     -1,      0}, {0xa646, 0xa646,      0,      1},
	{0xa647, 0xa647,     -1,      0}, {0xa648, 0xa648,      0,      1}, {0xa649, 0xa649,     -1,      0},
	{0xa64a, 0xa64a,      0,      1}, {0xa64b, 0xa64b,     -1,      0}, {0xa64c, 0xa64c,      0,      1},
	{0xa64d, 0xa64d,     -1,      0}, {0xa64e, 0xa64e,      0,      1}, {0xa64f, 0xa64f,     -1,      0},
	{0xa650, 0xa650,      0,      1}, {0xa651, 0xa651,     -1,      0}, {0xa652, 0xa652,      0,      1},
	{0xa653, 0xa653,     -1,      0}, {0xa654, 0xa654,      0,      1}, {0xa655, 0xa655,     -1,      0},
	{0xa656, 0xa656,      0,      1}, {0xa657, 0xa657,     -1,      0}, {0xa658, 0xa658,      0,      1},
	{0xa659, 0xa659,     -1,      0}, {0xa65a, 0xa65a,      0,      1}, {0xa65b, 0xa65b,     -1,      0},
	{0xa65c, 0xa65c,      0,      1}, {0xa65d, 0xa65d,     -1,      0}, {0xa65e, 0xa65e,      0,      1},
	{0xa65f, 0xa65f,     -1,      0}, {0xa660, 0xa660,      0,      1}, {0xa661, 0xa661,     -1,      0},
	{0xa662, 0xa662,      0,      1}, {0xa663, 0xa663,     -1,      0}, {0xa664, 0xa664,      0,      1},
	{0xa665, 0xa665,     -1,      0}, {0xa666, 0xa666,      0,      1}, {0xa667, 0xa667,     -1,      0},
	{0xa668, 0xa668,      0,      1}, {0xa669, 0xa669,     -1,      0}, {0xa66a, 0xa66a,      0,      1},
	{0xa66b, 0xa66b,     -1,      0}, {0xa66c, 0xa66c,      0,      1}, {0xa66d, 0xa66d,     -1,      0},
	{0xa680, 0xa680,      0,      1}, {0xa681, 0xa681,     -1,      0}, {0xa682, 0xa682,      0,      1},
	{0xa683, 0xa683,     -1,      0}, {0xa684, 0xa684,      0,      1}, {0xa685, 0xa685,     -1,      0},
	{0xa686, 0xa686,      0,      1}, {0xa687, 0xa687,     -1,      0}, {0xa688, 0xa688,      0,      1},
	{0xa689, 0xa689,     -1,      0}, {0xa68a, 0xa68a,      0,      1}, {0xa68b, 0xa68b,     -1,      0},
	{0xa68c, 0xa68c,      0,      1}, {0xa68d, 0xa68d,     -1,      0}, {0xa68e, 0xa68e,      0,      1},
	{0xa68f, 0xa68f,     -1,      0}, {0xa690, 0xa690,      0,      1}, {0xa691, 0xa691,     -1,      0},
	{0xa692, 0xa692,      0,      1}, {0xa693, 0xa693,     -1,      0}, {0xa694, 0xa694,      0,      1},
	{0xa695, 0xa695,     -1,      0}, {0xa696, 0xa696,      0,      1}, {0xa697, 0xa697,     -1,      0},
	{0xa698, 0xa698,      0,      1}, {0xa699, 0xa699,     -1,      0}, {0xa69a, 0xa69a,      0,      1},
	{0xa69b, 0xa69b,     -1,      0}, {0xa722, 0xa722,      0,      1}, {0xa723, 0xa723,     -1,      0},
	{0xa724, 0xa724,      0,      1}, {0xa725, 0xa725,     -1,      0}, {0xa726, 0xa726,      0,      1},
	{0xa727, 0xa727,     -1,      0}, {0xa728, 0xa728,      0,      1}, {0xa729, 0xa729,     -1,      0},
	{0xa72a, 0xa72a,      0,      1}, {0xa72b, 0xa72b,     -1,      0}, {0xa72c, 0xa72c,      0,      1},
	{0xa72d, 0xa72d,     -1,      0}, {0xa72e, 0xa72e,      0,      1}, {0xa72f, 0xa72f,     -1,      0},
	{0xa732, 0xa732,      0,      1}, {0xa733, 0xa733,     -1,      0}, {0xa734, 0xa734,      0,      1},
	{0xa735, 0xa735,     -1,      0}, {0xa736, 0xa736,      0,      1}, {0xa737, 0xa737,     -1,      0},
	{0xa738, 0xa738,      0,      1}, {0xa739, 0xa739,     -1,      0}, {0xa73a, 0xa73a,      0,      1},
	{0xa73b, 0xa73b,     -1,      0}, {0xa73c, 0xa73c,      0,      1}, {0xa73d, 0xa73d,     -1,      0},
	{0xa73e, 0xa73e,      0,      1}, {0xa73f, 0xa73f,     -1,      0}, {0xa740, 0xa740,      0,      1},
	{0xa741, 0xa741,     -1,      0}, {0xa742, 0xa742,      0,      1}, {0xa743, 0xa743,     -1,      0},
	{0xa744, 0xa744,      0,      1}, {0xa745, 0xa745,     -1,      0}, {0xa746, 0xa746,      0,      1},
	{0xa747, 0xa747,     -1,      0}, {0xa748, 0xa748,      0,      1}, {0xa749, 0xa749,     -1,      0},
	{0xa74a, 0xa74a,      0,      1}, {0xa74b, 0xa74b,     -1,      0}, {0xa74c, 0xa74c,      0,      1},
	{0xa74d, 0xa74d,     -1,      0}, {0xa74e, 0xa74e,      0,      1}, {0xa74f, 0xa74f,     -1,      0},
	{0xa750, 0xa750,      0,      1}, {0xa751, 0xa751,     -1,      0}, {0xa752, 0xa752,      0,      1},
	{0xa753, 0xa753,     -1,      0}, {0xa754, 0xa754,      0,      1}, {0xa755, 0xa755,     -1,      0},
	{0xa756, 0xa756,      0,      1}, {0xa757, 0xa757,     -1,      0}, {0xa758, 0xa758,      0,      1},
	{0xa759, 0xa759,     -1,      0}, {0xa75a, 0xa75a,      0,      1}, {0xa75b, 0xa75b,     -1,      0},
	{0xa75c, 0xa75c,      0,      1}, {0xa75d, 0xa75d,     -1,      0}, {0xa75e, 0xa75e,      0,      1},
	{0xa75f, 0xa75f,     -1,      0}, {0xa760, 0xa760,      0,      1}, {0xa761, 0xa761,     -1,      0},
	{0xa762, 0xa762,      0,      1}, {0xa763, 0xa763,     -1,      0}, {0xa764, 0xa764,      0,      1},
	{0xa765, 0xa765,     -1,      0}, {0xa766, 0xa766,      0,      1}, {0xa767, 0xa767,     -1,      0},
	{0xa768, 0xa768,      0,      1}, {0xa769, 0xa769,     -1,      0}, {0xa76a, 0xa76a,      0,      1},
	{0xa76b, 0xa76b,     -1,      0}, {0xa76c, 0xa76c,      0,      1}, {0xa76d, 0xa76d,     -1,      0},
	{0xa76e, 0xa76e,      0,      1}, {0xa76f, 0xa76f,     -1,      0}, {0xa779, 0xa779,      0,      1},
	{0xa77a, 0xa77a,     -1,      0}, {0xa77b, 0xa77b,      0,      1}, {0xa77c, 0xa77c,     -1,      0},
	{0xa77d, 0xa77d,      0, -35332}, {0xa77e, 0xa77e,      0,      1}, {0xa77f, 0xa77f,     -1,      0},
	{0xa780, 0xa780,      0,      1}, {0xa781, 0xa781,     -1,      0}, {0xa782, 0xa782,      0,      1},
	{0xa783, 0xa783,     -1,      0}, {0xa784, 0xa784,      0,      1}, {0xa785, 0xa785,     -1,      0},
	{0xa786, 0xa786,      0,      1}, {0xa787, 0xa787,     -1,      0}, {0xa78b, 0xa78b,      0,      1},
	{0xa78c, 0xa78c,     -1,      0}, {0xa78d, 0xa78d,      0, -42280}, {0xa790, 0xa790,      0,      1},
	{0xa791, 0xa791,     -1,      0}, {0xa792, 0xa792,      0,      1}, {0xa793, 0xa793,     -1,      0},
	{0xa794, 0xa794,     48,      0}, {0xa796, 0xa796,      0,      1}, {0xa797, 0xa797,     -1,      0},
	{0xa798, 0xa798,      0,      1}, {0xa799, 0xa799,     -1,      0}, {0xa79a, 0xa79a,      0,      1},
	{0xa79b, 0xa79b,     -1,      0}, {0xa79c, 0xa79c,      0,      1}, {0xa79d, 0xa79d,     -1,      0},
	{0xa79e, 0xa79e,      0,      1}, {0xa79f, 0xa79f,     -1,      0}, {0xa7a0, 0xa7a0,      0,      1},
	{0xa7a1, 0xa7a1,     -1,      0}, {0xa7a2, 0xa7a2,      0,      1}, {0xa7a3, 0xa7a3,     -1,      0},
	{0xa7a4, 0xa7a4,      0,      1}, {0xa7a5, 0xa7a5,     -1,      0}, {0xa7a6, 0xa7a6,      0,      1},
	{0xa7a7, 0xa7a7,     -1,      0}, {0xa7a8, 0xa7a8,      0,      1}, {0xa7a9, 0xa7a9,     -1,      0},
	{0xa7aa, 0xa7aa,      0, -42308}, {0xa7ab, 0xa7ab,      0, -42319}, {0xa7ac, 0xa7ac,      0, -42315},
	{0xa7ad, 0xa7ad,      0, -42305}, {0xa7ae, 0xa7ae,      0, -42308}, {0xa7b0, 0xa7b0,      0, -42258},
	{0xa7b1, 0xa7b1,      0, -42282}, {0xa7b2, 0xa7b2,      0, -42261}, {0xa7b3, 0xa7b3,      0,    928},
	{0xa7b4, 0xa7b4,      0,      1}, {0xa7b5, 0xa7b5,     -1,      0}, {0xa7b6, 0xa7b6,      0,      1},
	{0xa7b7, 0xa7b7,     -1,      0}, {0xa7b8, 0xa7b8,      0,      1}, {0xa7b9, 0xa7b9,     -1,      0},
	{0xa7ba, 0xa7ba,      0,      1}, {0xa7bb, 0xa7bb,     -1,      0}, {0xa7bc, 0xa7bc,      0,      1},
	{0xa7bd, 0xa7bd,     -1,      0}, {0xa7be, 0xa7be,      0,      1}, {0xa7bf, 0xa7bf,     -1,      0},
	{0xa7c0, 0xa7c0,      0,      1}, {0xa7c1, 0xa7c1,     -1,      0}, {0xa7c2, 0xa7c2,      0,      1},
	{0xa7c3, 0xa7c3,     -1,      0}, {0xa7c4, 0xa7c4,      0,    -48}, {0xa7c5, 0xa7c5,      0, -42307},
	{0xa7c6, 0xa7c6,      0, -35384}, {0xa7c7, 0xa7c7,      0,      1}, {0xa7c8, 0xa7c8,     -1,      0},
	{0xa7c9, 0xa7c9,      0,      1}, {0xa7ca, 0xa7ca,     -1,      0}, {0xa7cb, 0xa7cb,      0, -42343},
	{0xa7cc, 0xa7cc,      0,      1}, {0xa7cd, 0xa7cd,     -1,      0}, {0xa7ce, 0xa7ce,      0,      1},
	{0xa7cf, 0xa7cf,     -1,      0}, {0xa7d0, 0xa7d0,      0,      1}, {0xa7d1, 0xa7d1,     -1,      0},
	{0xa7d2, 0xa7d2,      0,      1}, {0xa7d3, 0xa7d3,     -1,      0}, {0xa7d4, 0xa7d4,      0,      1},
	{0xa7d5, 0xa7d5,     -1,      0}, {0xa7d6, 0xa7d6,      0,      1}, {0xa7d7, 0xa7d7,     -1,      0},
	{0xa7d8, 0xa7d8,      0,      1}, {0xa7d9, 0xa7d9,     -1,      0}, {0xa7da, 0xa7da,      0,      1},
	{0xa7db, 0xa7db,     -1,      0}, {0xa7dc, 0xa7dc,      0, -42561}, {0xa7dd, 0xa7dd,      0, -42342},
	{0xa7e2, 0xa7e2,      0, -42342}, {0xa7f5, 0xa7f5,      0,      1}, {0xa7f6, 0xa7f6,     -1,      0},
	{0xab4b, 0xab4c,     33,      0}, {0xab53, 0xab53,   -928,      0}, {0xab6c, 0xab6d,      0,    -33},
	{0xab70, 0xabbf, -38864,      0}, {0xff21, 0xff3a,      0,     32}, {0xff41, 0xff5a,    -32,      0}
};


//
//	case32
//

#if defined(IMGUI_USE_WCHAR32)

static CaseRange32 case32[] = {
	{0x10400, 0x10427,   0, 40}, {0x10428, 0x1044f, -40,  0}, {0x104b0, 0x104d3,   0, 40}, {0x104d8, 0x104fb, -40,  0},
	{0x10570, 0x1057a,   0, 39}, {0x1057c, 0x1058a,   0, 39}, {0x1058c, 0x10592,   0, 39}, {0x10594, 0x10595,   0, 39},
	{0x10597, 0x105a1, -39,  0}, {0x105a3, 0x105b1, -39,  0}, {0x105b3, 0x105b9, -39,  0}, {0x105bb, 0x105bc, -39,  0},
	{0x10c80, 0x10cb2,   0, 64}, {0x10cc0, 0x10cf2, -64,  0}, {0x10d50, 0x10d65,   0, 32}, {0x10d70, 0x10d85, -32,  0},
	{0x118a0, 0x118bf,   0, 32}, {0x118c0, 0x118df, -32,  0}, {0x16e40, 0x16e5f,   0, 32}, {0x16e60, 0x16e7f, -32,  0},
	{0x16ea0, 0x16eb8,   0, 27}, {0x16ebb, 0x16ed3, -27,  0}, {0x1df40, 0x1df40,   0,  1}, {0x1df41, 0x1df41,  -1,  0},
	{0x1df48, 0x1df48,   0,  1}, {0x1df49, 0x1df49,  -1,  0}, {0x1df4a, 0x1df4a,   0,  1}, {0x1df4b, 0x1df4b,  -1,  0},
	{0x1df4d, 0x1df4d,   0,  1}, {0x1df4e, 0x1df4e,  -1,  0}, {0x1df51, 0x1df51,   0,  1}, {0x1df52, 0x1df52,  -1,  0},
	{0x1df68, 0x1df68,   0,  1}, {0x1df69, 0x1df69,  -1,  0}, {0x1df6a, 0x1df6a,   0,  1}, {0x1df6b, 0x1df6b,  -1,  0},
	{0x1df6c, 0x1df6c,   0,  1}, {0x1df6d, 0x1df6d,  -1,  0}, {0x1df6e, 0x1df6e,   0,  1}, {0x1df6f, 0x1df6f,  -1,  0},
	{0x1df72, 0x1df72,   0,  1}, {0x1df73, 0x1df73,  -1,  0}, {0x1df74, 0x1df74,   0,  1}, {0x1df75, 0x1df75,  -1,  0},
	{0x1df76, 0x1df76,   0,  1}, {0x1df77, 0x1df77,  -1,  0}, {0x1df78, 0x1df78,   0,  1}, {0x1df79, 0x1df79,  -1,  0},
	{0x1df7a, 0x1df7a,   0,  1}, {0x1df7b, 0x1df7b,  -1,  0}, {0x1df7c, 0x1df7c,   0,  1}, {0x1df7d, 0x1df7d,  -1,  0},
	{0x1df7e, 0x1df7e,   0,  1}, {0x1df7f, 0x1df7f,  -1,  0}, {0x1e900, 0x1e921,   0, 34}, {0x1e922, 0x1e943, -34,  0}
};

#endif


//
//	Internal type conversions because "char" is signed
//

static inline ImWchar uch(char c) {
	return static_cast<ImWchar>(c);
}

static inline char sch(ImWchar i) {
	return static_cast<char>(i);
}


//
//	skipBOM
//

std::string_view::const_iterator TextEditor::CodePoint::skipBOM(std::string_view::const_iterator i, std::string_view::const_iterator end) {
	// skip Byte Order Mark (BOM) just in case there is one

	// Note: the standard states that:
	// Use of a BOM is neither required nor recommended for UTF-8
	static constexpr char bom1 = static_cast<char>(0xEF);
	static constexpr char bom2 = static_cast<char>(0xBB);
	static constexpr char bom3 = static_cast<char>(0xBF);
	return ((end - i) >= 3 && i[0] == bom1 && i[1] == bom2 && i[2] == bom3) ? i + 3 : i;
}


//
//	TextEditor::CodePoint::read
//

std::string_view::const_iterator TextEditor::CodePoint::read(std::string_view::const_iterator i, std::string_view::const_iterator end, ImWchar *codepoint) {
	// parse a UTF-8 sequence into a unicode codepoint and return updated iterator
	if (i < end && (uch(*i) & 0x80) == 0) {
		*codepoint = uch(*i);
		++i;

	} else if (i + 1 < end && (uch(*i) & 0xE0) == 0xC0) {
		*codepoint = ((uch(*i) & 0x1f) << 6) | (uch(*(i + 1)) & 0x3f);
		i += 2;

	} else if (i + 2 < end && (uch(*i) & 0xF0) == 0xE0) {
		*codepoint = ((uch(*i) & 0x0f) << 12) | ((uch(*(i + 1)) & 0x3f) << 6) | (uch(*(i + 2)) & 0x3f);
		i += 3;

	} else if (i + 3 < end && (uch(*i) & 0xF8) == 0xF0) {
#if defined(IMGUI_USE_WCHAR32)
		*codepoint = ((uch(*i) & 0x07) << 18) | ((uch(*(i + 1)) & 0x3f) << 12) | ((uch(*(i + 2)) & 0x3f) << 6) | (uch(*(i + 3)) & 0x3f);
#else
		*codepoint = IM_UNICODE_CODEPOINT_INVALID;
#endif
		i += 4;

	} else {
		*codepoint = IM_UNICODE_CODEPOINT_INVALID;
		++i;
	}

	return i;
}


//
//	TextEditor::CodePoint::write
//

size_t TextEditor::CodePoint::write(char* start, ImWchar codepoint) {
	// generate UTF-8 sequence from a unicode codepoint and return bytes written
	auto i = start;

	if (codepoint < 0x80) {
		*i++ = sch(codepoint);

	} else if (codepoint < 0x800) {
		*i++ = sch(0xc0 | ((codepoint >> 6) & 0x1f));
		*i++ = sch(0x80 | (codepoint & 0x3f));

#if defined(IMGUI_USE_WCHAR32)
	} else if (codepoint < 0x10000) {
		*i++ = sch(0xe0 | ((codepoint >> 12) & 0x0f));
		*i++ = sch(0x80 | ((codepoint >> 6) & 0x3f));
		*i++ = sch(0x80 | (codepoint & 0x3f));

	} else if (codepoint >= 0x110000) {
		codepoint = IM_UNICODE_CODEPOINT_INVALID;
		*i++ = sch(0xe0 | ((codepoint >> 12) & 0x0f));
		*i++ = sch(0x80 | ((codepoint >> 6) & 0x3f));
		*i++ = sch(0x80 | (codepoint & 0x3f));

	} else {
		*i++ = sch(0xf0 | ((codepoint >> 18) & 0x07));
		*i++ = sch(0x80 | ((codepoint >> 12) & 0x3f));
		*i++ = sch(0x80 | ((codepoint >> 6) & 0x3f));
		*i++ = sch(0x80 | (codepoint & 0x3f));

#else
	} else {
		*i++ = sch(0xe0 | ((codepoint >> 12) & 0x0f));
		*i++ = sch(0x80 | ((codepoint >> 6) & 0x3f));
		*i++ = sch(0x80 | (codepoint & 0x3f));
#endif
	}

	return i - start;
}


//
//	rangeContains
//

template <typename T, typename C>
bool rangeContains(const T& table, C codepoint) {
	auto low = std::begin(table);
	auto high = std::end(table);

	while (low <= high) {
		auto mid = low + (high - low) / 2;

		if (codepoint >= mid->low && codepoint <= mid->high) {
			return (mid->stride == 1) || ((codepoint - mid->low) % mid->stride == 0);

		} else if (codepoint < mid->low) {
			high = mid - 1;

		} else {
			low = mid + 1;
		}
	}

	return false;
}


//
//	TextEditor::CodePoint::isLetter
//

bool TextEditor::CodePoint::isLetter(ImWchar codepoint) {
	if (codepoint < 0x7f) {
		return static_cast<unsigned>((codepoint | 32) - 'a') < 26;

#if defined(IMGUI_USE_WCHAR32)
	} else if (codepoint >= 0x10000) {
		return rangeContains(letters32, static_cast<ImWchar32>(codepoint));
#endif

	} else {
		return rangeContains(letters16, static_cast<ImWchar16>(codepoint));
	}
}


//
//	TextEditor::CodePoint::isNumber
//

bool TextEditor::CodePoint::isNumber(ImWchar codepoint) {
	if (codepoint < 0x7f) {
		return static_cast<unsigned>(codepoint - '0') < 10;

#if defined(IMGUI_USE_WCHAR32)
	} else if (codepoint >= 0x10000) {
		return rangeContains(numbers32, static_cast<ImWchar32>(codepoint));
#endif

	} else {
		return rangeContains(numbers16, static_cast<ImWchar16>(codepoint));
	}
}


//
//	TextEditor::CodePoint::isXidStart
//

bool TextEditor::CodePoint::isXidStart(ImWchar codepoint) {
	if (codepoint < 0x7f) {
		return codepoint == '_' || static_cast<unsigned>((codepoint | 32) - 'a') < 26;

#if defined(IMGUI_USE_WCHAR32)
	} else if (codepoint >= 0x10000) {
		return rangeContains(xidStart32, static_cast<ImWchar32>(codepoint));
#endif

	} else {
		return rangeContains(xidStart16, static_cast<ImWchar16>(codepoint));
	}
}


//
//	TextEditor::CodePoint::isWhiteSpace
//

bool TextEditor::CodePoint::isWhiteSpace(ImWchar codepoint) {
	if (codepoint < 0x7f) {
		return codepoint == ' ' || static_cast<unsigned>(codepoint - '\t') < 5;

#if defined(IMGUI_USE_WCHAR32)
	} else if (codepoint >= 0x10000) {
		return false;
#endif

	} else {
		return rangeContains(whitespace16, static_cast<ImWchar16>(codepoint));
	}
}


//
//	TextEditor::CodePoint::isWord
//

bool TextEditor::CodePoint::isWord(ImWchar codepoint) {
	if (codepoint < 0x7f) {
		return
			(static_cast<unsigned>((codepoint | 32) - 'a') < 26) ||
			(static_cast<unsigned>(codepoint - '0') < 10) ||
			codepoint == '_';

#if defined(IMGUI_USE_WCHAR32)
	} else if (codepoint >= 0x10000) {
		return
			rangeContains(letters32, static_cast<ImWchar32>(codepoint)) ||
			rangeContains(numbers32, static_cast<ImWchar32>(codepoint)) ||
			codepoint == '_';
#endif

	} else {
		return
			rangeContains(letters16, static_cast<ImWchar16>(codepoint)) ||
			rangeContains(numbers16, static_cast<ImWchar16>(codepoint)) ||
			codepoint == '_';
	}
}


//
//	TextEditor::CodePoint::isXidContinue
//

bool TextEditor::CodePoint::isXidContinue(ImWchar codepoint) {
	if (codepoint < 0x7f) {
		return codepoint == '_' || (static_cast<unsigned>((codepoint | 32) - 'a') < 26) || (static_cast<unsigned>(codepoint - '0') < 10);

#if defined(IMGUI_USE_WCHAR32)
	} else if (codepoint >= 0x10000) {
		return rangeContains(xidContinue32, static_cast<ImWchar32>(codepoint));
#endif

	} else {
		return rangeContains(xidContinue16, static_cast<ImWchar16>(codepoint));
	}
}


//
//	TextEditor::CodePoint::isLower
//

bool TextEditor::CodePoint::isLower(ImWchar codepoint) {
	if (codepoint < 0x7f) {
		return static_cast<unsigned>(codepoint - 'a') < 26;

#if defined(IMGUI_USE_WCHAR32)
	} else if (codepoint >= 0x10000) {
		return rangeContains(lower32, static_cast<ImWchar32>(codepoint));
#endif

	} else {
		return rangeContains(lower16, static_cast<ImWchar16>(codepoint));
	}
}


//
//	TextEditor::CodePoint::isUpper
//

bool TextEditor::CodePoint::isUpper(ImWchar codepoint) {
	if (codepoint < 0x7f) {
		return static_cast<unsigned>(codepoint - 'A') < 26;

#if defined(IMGUI_USE_WCHAR32)
	} else if (codepoint >= 0x10000) {
		return rangeContains(upper32, static_cast<ImWchar32>(codepoint));
#endif

	} else {
		return rangeContains(upper16, static_cast<ImWchar16>(codepoint));
	}
}


//
//	rangeFind
//

template <typename T, typename C>
bool rangeFind(const T& table, C codepoint) {
	auto low = std::begin(table);
	auto high = std::end(table);

	while (low <= high) {
		auto mid = low + (high - low) / 2;

		if (codepoint >= mid->low && codepoint <= mid->high) {
			return true;

		} else if (codepoint < mid->low) {
			high = mid - 1;

		} else {
			low = mid + 1;
		}
	}

	return false;
}


//
//	isEastAsian
//

bool TextEditor::CodePoint::isEastAsian(ImWchar codepoint) {
	// handle simple case
	if (codepoint < 0x1100) {
		return false;
	}

	bool result;

#if defined(IMGUI_USE_WCHAR32)
	if (codepoint >= 0x10000) {
		result = rangeFind(eastAsian32, static_cast<char32_t>(codepoint));

	} else
#endif

	{
		result = rangeFind(eastAsian16, static_cast<char16_t>(codepoint));
	}

	if (!result) {
		if ((codepoint >= 0x3400 && codepoint <= 0x4DBF) ||
			(codepoint >= 0x4E00 && codepoint <= 0x9FFF)) {

			result = true;
		}
	}

	return result;
}


//
//	TextEditor::CodePoint::getGlyphWidth
//

size_t TextEditor::CodePoint::getGlyphWidth(ImWchar codepoint) {
	// handle simple case
	if (codepoint < 0x1100) {
		return 1;
	}

	bool wide;

#if defined(IMGUI_USE_WCHAR32)
	if (codepoint >= 0x10000) {
		wide = rangeFind(wideGlyph32, static_cast<char32_t>(codepoint));

	} else
#endif

	{
		wide = rangeFind(wideGlyph16, static_cast<char16_t>(codepoint));
	}

	if (!wide) {
		if ((codepoint >= 0x3400 && codepoint <= 0x4DBF) ||
			(codepoint >= 0x4E00 && codepoint <= 0x9FFF)) {

			wide = true;
		}
	}

	return wide ? 2 : 1;
}


//
//	caseRangeFind
//

template <typename T, typename C>
const CaseRange<C>* caseRangeFind(const T& table, C codepoint) {
	auto low = std::begin(table);
	auto high = std::end(table);

	while (low <= high) {
		auto mid = low + (high - low) / 2;

		if (codepoint >= mid->low && codepoint <= mid->high) {
			return mid;

		} else if (codepoint < mid->low) {
			high = mid - 1;

		} else {
			low = mid + 1;
		}
	}

	return nullptr;
}


//
//	caseRangeToUpper
//

template <typename T, typename C>
C caseRangeToUpper(const T& table, C codepoint) {
	const auto caseRange = caseRangeFind(table, codepoint);

	if (!caseRange || caseRange->toUpper == 0) {
		return codepoint;

	} else if (caseRange->toUpper == 0xffff) {
		return codepoint & ~0x1;

	} else {
		return static_cast<C>(static_cast<int32_t>(codepoint) + caseRange->toUpper);
	}
}


//
//	caseRangeToLower
//

template <typename T, typename C>
C caseRangeToLower(const T& table, C codepoint) {
	const auto caseRange = caseRangeFind(table, codepoint);

	if (!caseRange || caseRange->toLower == 0) {
		return codepoint;

	} else if (caseRange->toLower == 0xffff) {
		return codepoint | 0x1;

	} else {
		return static_cast<C>(static_cast<int32_t>(codepoint) + caseRange->toLower);
	}
}


//
//	TextEditor::CodePoint::toUpper
//

ImWchar TextEditor::CodePoint::toUpper(ImWchar codepoint) {
	if (codepoint < 0x7f) {
		return (static_cast<unsigned>(codepoint - 'a') < 26) ? codepoint & 0x5f : codepoint;

#if defined(IMGUI_USE_WCHAR32)
	} else if (codepoint >= 0x10000) {
		return caseRangeToUpper(case32, static_cast<char32_t>(codepoint));
#endif

	} else {
		return caseRangeToUpper(case16, static_cast<char16_t>(codepoint));
	}
}


//
//	TextEditor::CodePoint::toLower
//

ImWchar TextEditor::CodePoint::toLower(ImWchar codepoint) {
	if (codepoint < 0x7f) {
		return (static_cast<unsigned>(codepoint - 'A') < 26) ? codepoint | 32 : codepoint;

#if defined(IMGUI_USE_WCHAR32)
	} else if (codepoint >= 0x10000) {
		return caseRangeToLower(case32, static_cast<char32_t>(codepoint));
#endif

	} else {
		return caseRangeToLower(case16, static_cast<char16_t>(codepoint));
	}
}


//
//	TextEditor::updatePalette
//

void TextEditor::updatePalettes() {
	// update palette with the current alpha from the Dear ImGui style
	for (size_t i = 0; i < static_cast<size_t>(Color::count); i++) {
		palette[i] = ImGui::GetColorU32(paletteBase[i]);
		miniMapPalette[i] = ImGui::GetColorU32(paletteBase[i], miniMapAlpha);
	}
}


//
//	Color palettes
//

const TextEditor::Palette& TextEditor::GetDarkPalette() {
	const static Palette palette = {{
		IM_COL32(224, 224, 224, 255),	// text
		IM_COL32(197, 134, 192, 255),	// keyword
		IM_COL32( 90, 179, 155, 255),	// declaration
		IM_COL32(181, 206, 168, 255),	// number
		IM_COL32(206, 145, 120, 255),	// string
		IM_COL32(255, 255, 153, 255),	// punctuation
		IM_COL32( 64, 192, 128, 255),	// preprocessor
		IM_COL32(156, 220, 254, 255),	// identifier
		IM_COL32( 79, 193, 255, 255),	// known identifier
		IM_COL32(106, 153,  85, 255),	// comment
		IM_COL32( 30,  30,  30, 255),	// background
		IM_COL32(224, 224, 224, 255),	// cursor
		IM_COL32( 32,  96, 160, 255),	// selection
		IM_COL32( 80,  80,  80, 255),	// whitespace
		IM_COL32( 70,  70,  70, 255),	// matchingBracketBackground
		IM_COL32(140, 140, 140, 255),	// matchingBracketActive
		IM_COL32(246, 222,  36, 255),	// matchingBracketLevel1
		IM_COL32( 66, 120, 198, 255),	// matchingBracketLevel2
		IM_COL32(213,  96, 213, 255),	// matchingBracketLevel3
		IM_COL32(198,   8,  32, 255),	// matchingBracketError
		IM_COL32(128, 128, 144, 255),	// line number
		IM_COL32(224, 224, 240, 255),	// current line number
		IM_COL32(255, 255, 255,   8),	// current line highlight
		IM_COL32(255, 255, 255,  16),	// current line highlight border

		IM_COL32(255,   0,   0, 255),	// custom color 1
		IM_COL32(245, 140,   0, 255),	// custom color 2
		IM_COL32(255, 255,  40, 255),	// custom color 3
		IM_COL32(  0, 255,   0, 255),	// custom color 4
		IM_COL32( 63, 255, 205, 255),	// custom color 5
		IM_COL32( 70, 240, 240, 255),	// custom color 6
		IM_COL32(  0,   0, 255, 255),	// custom color 7
		IM_COL32(145,   0, 224, 255),	// custom color 8
		IM_COL32(240,  50, 230, 255)	// custom color 9
}};

	return palette;
}

const TextEditor::Palette& TextEditor::GetLightPalette() {
	const static Palette palette = {{
		IM_COL32( 64,  64,  64, 255),	// text
		IM_COL32( 170,  0, 220, 255),	// keyword
		IM_COL32( 65,   0, 255, 255),	// declaration
		IM_COL32( 40, 140,  90, 255),	// number
		IM_COL32(160,  32,  32, 255),	// string
		IM_COL32(  0,   0,   0, 255),	// punctuation
		IM_COL32( 96,  96,  64, 255),	// preprocessor
		IM_COL32( 64,  64,  64, 255),	// identifier
		IM_COL32( 16,  96,  96, 255),	// known identifier
		IM_COL32( 35, 135,   5, 255),	// comment
		IM_COL32(255, 255, 255, 255),	// background
		IM_COL32(  0,   0,   0, 255),	// cursor
		IM_COL32(  0,   0,  96,  64),	// selection
		IM_COL32(144, 144, 144, 144),	// whitespace
		IM_COL32(180, 180, 180, 144),	// matchingBracketBackground
		IM_COL32( 72,  72,  72, 255),	// matchingBracketActive
		IM_COL32( 70,   0, 250, 255),	// matchingBracketLevel1
		IM_COL32( 80, 160,  70, 255),	// matchingBracketLevel2
		IM_COL32(120,  60,  25, 255),	// matchingBracketLevel3
		IM_COL32(198,   8,  32, 255),	// matchingBracketError
		IM_COL32(  0,  80,  80, 255),	// line number
		IM_COL32(  0,   0,   0, 255),	// current line number
		IM_COL32(  0,   0,   0,   8),	// current line highlight
		IM_COL32(  0,   0,   0,  16),	// current line highlight border

		IM_COL32(255,   0,   0, 255),	// custom color 1
		IM_COL32(245, 140,   0, 255),	// custom color 2
		IM_COL32(255, 255,  40, 255),	// custom color 3
		IM_COL32(  0, 255,   0, 255),	// custom color 4
		IM_COL32( 63, 255, 205, 255),	// custom color 5
		IM_COL32( 70, 240, 240, 255),	// custom color 6
		IM_COL32(  0,   0, 255, 255),	// custom color 7
		IM_COL32(145,   0, 224, 255),	// custom color 8
		IM_COL32(240,  50, 230, 255)	// custom color 9
	}};

	return palette;
}

TextEditor::Palette TextEditor::defaultPalette = TextEditor::GetDarkPalette();


//
//	Include language definitions
//

#include "TextEditorLanguages.h"


//
//	TextEditor::SetLanguage
//

void TextEditor::SetLanguage(const Language* language) {
	config.language = language;

	if (languageChangeCallback) {
		languageChangeCallback();
	}
}


//
//	TextEditor::RegisterLanguage
//

void TextEditor::RegisterLanguage(const std::string name, std::function<const Language*()> creator, const std::vector<std::string> extensions) {
	builtinLanguages.emplace_back(name);
	builtinLanguagesIndex.emplace(name);
	std::sort(builtinLanguages.begin(), builtinLanguages.end());
	builtinLanguageCreators[name] = creator;

	for (auto& extension : extensions) {
		builtinLanguageExtensions[extension] = creator;
	}
}


//
//	TextEditor::LookupLanguage
//

const TextEditor::Language* TextEditor::LookupLanguage(const std::string& name) {
	if (!languagesRegistered) {
		TextEditorLanguages::registerLanguages();
		languagesRegistered = true;
	}

	if (builtinLanguagesIndex.find(name) == builtinLanguagesIndex.end()) {
		return nullptr;

	} else {
		return builtinLanguageCreators[name]();
	}
}


//
//	TextEditor::LookupLanguageByExtension
//

const TextEditor::Language* TextEditor::LookupLanguageByExtension(const std::string& extension) {
	if (!languagesRegistered) {
		TextEditorLanguages::registerLanguages();
		languagesRegistered = true;
	}

	if (builtinLanguageExtensions.find(extension) == builtinLanguageExtensions.end()) {
		return nullptr;

	} else {
		return builtinLanguageExtensions[extension]();
	}
}


//
//	TextEditor::GetRegisteredLanguages
//

const std::vector<std::string> TextEditor::GetRegisteredLanguages() {
	if (!languagesRegistered) {
		TextEditorLanguages::registerLanguages();
		languagesRegistered = true;
	}

	return builtinLanguages;
}
