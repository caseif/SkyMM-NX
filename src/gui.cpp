#include "console_helper.hpp"
#include "error_defs.hpp"
#include "gui.hpp"

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)
#define CLAMP(n, l, h) (MIN(MAX(n, l), h))

void ModGui::scrollSelection(int delta) {
    //TODO
}

void ModGui::redraw(void) {
    for (size_t y = 0; y < display_rows; y++) {
        redrawRow(y);
    }
}

void ModGui::redrawRow(size_t gui_y) {
    if (gui_y >= display_rows) {
        PANIC();
    }

    size_t screen_y = guiToScreenSpace(gui_y);
    size_t list_index = guiToListSpace(gui_y);
    std::shared_ptr<SkyrimMod> cur_mod = mod_list.at(list_index);

    bool highlighted = selected_row == list_index;

    CONSOLE_SET_POS(0, 0);
    for (size_t i = 0; i < screen_y; i++) {
        CONSOLE_MOVE_DOWN(1);
    }
    CONSOLE_CLEAR_LINE();

    printf("[");

    ModStatus mod_status = cur_mod->getStatus();
    switch (mod_status) {
        case ModStatus::ENABLED:
            CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_GREEN);
            printf("*");
            if (highlighted) {
                CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_BLACK);
            } else {
                CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_WHITE);
            }
            break;
        case ModStatus::PARTIAL:
            CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_YELLOW);
            printf("*");
            if (highlighted) {
                CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_BLACK);
            } else {
                CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_WHITE);
            }
            break;
        case ModStatus::DISABLED:
            printf(" ");
            break;
        default:
            PANIC();
    }

    printf("] ");

    printf(cur_mod->base_name.c_str());
    printf("\n");

    if (highlighted) {
        CONSOLE_SET_ATTRS(CONSOLE_ATTR_BOLD);
        CONSOLE_SET_COLOR(CONSOLE_COLOR_BG_BLACK);
        CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_WHITE);
    }
}
