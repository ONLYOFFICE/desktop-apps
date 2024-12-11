#ifndef OBJECT_H
#define OBJECT_H

#include <string>


class Object
{
public:
    Object(Object *parent = nullptr);
    virtual ~Object();

    enum ObjectType : unsigned char {
        ApplicationType,
        WindowType,
        DialogType,
        WidgetType,
        PopupType
    };

    Object *parent();
    void setParent(Object*);
    void setObjectName(const std::wstring&);
    std::wstring objectName();
    virtual void disconnect(int);

protected:
    static int m_connectionId;

private:
    Object      *m_parent;
    std::wstring m_object_name;
};

#endif // OBJECT_H
