/*

    settingskeys.h

        to avoid memory and typo issues when setting and getting settings

*/

#pragma once
#define setting static const char*


namespace Keys {
    namespace Prefs {
        setting tabsVsSpaces = "prefs:tabsVsSpaces";
        setting tabSize = "prefs:tabsize";
        setting autoCompleteChars = "prefs:autoCompleteChars";
        setting indentGuides = "prefs:indentationGuides";
    }
    namespace Editor {
        setting theme = "editorTheme";
    }
}
