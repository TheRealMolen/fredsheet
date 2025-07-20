#include "ui.h"

#include <iostream>
#include <string>
#include <sstream>


namespace json
{
    struct JsonStr
    {
        const char* Str = nullptr;

        JsonStr() = delete;
        explicit JsonStr(const char* s) : Str(s) { /**/ }
        explicit JsonStr(const std::string& s) : Str(s.c_str()) { /**/ }
    };
    std::ostream& operator<<(std::ostream& os, const JsonStr& str)
    {
        if (!str.Str)
        {
            os << "null";
            return os;
        }

        os << '"';

        // TODO: FIXME: UTF-8
        for (const char* pc = str.Str; *pc; ++pc)
        {
            const char c = *pc;
            switch (c)
            {
                case '\\':
                    os << "\\\\";
                    break;

                case '"':
                    os << "\\\"";
                    break;

                case '\n':
                    os << "\\n";

                default:
                    os << char(c);
            }
        }

        os << '"';

        return os;
    }

    struct JsonBool
    {
        bool Val = false;

        JsonBool() = delete;
        JsonBool(bool b) : Val(b) { /**/ }
    };
    std::ostream& operator<<(std::ostream& os, const JsonBool& b)
    {
        os << (b.Val ? "true" : "false");
        return os;
    }


    std::ostream & operator<<(std::ostream& os, const Vector2& v)
    {
        os << "[" << v.x << ", " << v.y << "]";
        return os;
    }
    std::ostream & operator<<(std::ostream& os, const Rectangle& rect)
    {
        os << "[" << rect.x << ", " << rect.y << ", " << rect.width << ", " << rect.height << "]";
        return os;
    }
}

namespace ui
{
    void DumpHierarchyToJson(std::ostream& os, const ControlPtr& ctrlP, const std::string& outerIndent)
    {
        using namespace json;

        const ControlState* ctrl = ctrlP.get();
        if (!ctrl)
        {
            os << "null";
            return;
        }

        const std::string indent = outerIndent + "  ";
        os << "{\n";
        os << indent << "\"DbgName\": " << JsonStr(ctrl->DbgName);
        os << ",\n" << indent << "\"Text\": " << JsonStr(ctrl->Text);
        os << ",\n" << indent << "\"IsFixedSize\": " << JsonBool(ctrl->IsFixedSize);
        os << ",\n" << indent << "\"IsHovered\": " << JsonBool(ctrl->IsHovered);
        os << ",\n" << indent << "\"IsSelected\": " << JsonBool(ctrl->IsSelected);
        os << ",\n" << indent << "\"IsFocused\": " << JsonBool(ctrl->IsFocused);
        os << ",\n" << indent << "\"MinDesiredSize\": " << ctrl->MinDesiredSize;
        os << ",\n" << indent << "\"DesiredSizeDirty\": " << JsonBool(ctrl->DesiredSizeDirty);
        os << ",\n" << indent << "\"Rect\": " << ctrl->Rect;

        if (!ctrl->Children.empty())
        {
            const std::string innerIndent = indent + "  ";
            os << ",\n" << indent << "\"Children\": [";
            bool firstChild = true;
            for (const ControlPtr& childP : ctrl->Children)
            {
                if (!firstChild)
                    os << ',';
                firstChild = false;

                os << '\n' << innerIndent;
                DumpHierarchyToJson(os, childP, innerIndent);
            }
            os << '\n' << indent << ']';
        }

        os << "\n" << outerIndent << '}';
    }


    std::string DumpHierarchyToJson(const ControlPtr& ctrl)
    {
        std::ostringstream os;
        DumpHierarchyToJson(os, ctrl, {});
        return os.str();
    }
};

