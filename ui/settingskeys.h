/*

    settingskeys.h

        to avoid memory and typo issues when setting and getting settings

*/

#pragma once


using Key = const char*;

namespace Keys {
    namespace Prefs {
        static Key tabsVsSpaces = "prefs:tabsVsSpaces";
        static Key tabSize = "prefs:tabsize";
        static Key autoCompleteChars = "prefs:autoCompleteChars";
        static Key indentGuides = "prefs:indentationGuides";
        static Key showLineNumbers = "prefs:showLineNumbers";
        static Key backspaceUnindents = "prefs:backspaceUnindents";
        static Key whitespaceVisibility = "prefs:whitespacevisibility";
		static Key foldCompact = "prefs:foldCompact";
    }
    namespace Editor {
        static Key theme = "editorTheme";
    }
}
