/*
 * This file is a part of SkyrimNXModManager.
 * Copyright (c) 2019, Max Roncace <mproncace@gmail.com>
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include "mod.hpp"

#include <memory>
#include <string>
#include <vector>

class ModGui {
    private:
        std::vector<std::shared_ptr<SkyrimMod>> &mod_list;
        size_t screen_off_y;
        size_t display_rows;
        size_t selected_row;
        size_t scroll;

        inline size_t listToGuiSpace(size_t list_index) {
            return list_index - scroll;
        }

        inline size_t listToScreenSpace(size_t list_index) {
            return guiToScreenSpace(listToGuiSpace(list_index));
        }

        inline size_t guiToListSpace(size_t gui_y) {
            return gui_y + scroll;
        }

        inline size_t guiToScreenSpace(size_t gui_y) {
            return gui_y + screen_off_y;
        }

        inline size_t screenToListSpace(size_t screen_y) {
            return guiToListSpace(screenToGuiSpace(screen_y));
        }

        inline size_t screenToGuiSpace(size_t screen_y) {
            return screen_y - screen_off_y;
        }

    public:
        ModGui(std::vector<std::shared_ptr<SkyrimMod>> &mod_list, size_t screen_off_y, size_t display_rows):
                mod_list(mod_list),
                screen_off_y(screen_off_y),
                display_rows(display_rows),
                selected_row(0),
                scroll(0) {
        }

        void scrollSelection(int delta);

        void redraw(void);

        void redrawRow(size_t row);
};
