/*

    settingskeys.h

        to avoid memory and typo issues when setting and getting settings

*/

#pragma once
#define KEY static const char*


namespace Keys {
    namespace Prefs {
        KEY tabsVsSpaces = "prefs:tabsVsSpaces";
        KEY tabSize = "prefs:tabsize";
        KEY autoCompleteChars = "prefs:autoCompleteChars";
        KEY indentGuides = "prefs:indentationGuides";
        KEY showLineNumbers = "prefs:showLineNumbers";
        KEY backspaceUnindents = "prefs:backspaceUnindents";
        KEY whitespaceVisibility = "prefs:whitespacevisibility";
    }
    namespace Editor {
        KEY theme = "editorTheme";
    }
}
