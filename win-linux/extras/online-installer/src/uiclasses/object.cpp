#include "object.h"


int Object::m_connectionId = 0;

Object::Object(Object *parent) :
    m_parent(parent)
{

}

Object::~Object()
{

}

Object *Object::parent()
{
    return m_parent;
}

void Object::setParent(Object *parent)
{
    m_parent = parent;
}

void Object::setObjectName(const std::wstring &object_name)
{
    m_object_name = object_name;
}

std::wstring Object::objectName()
{
    return m_object_name;
}

void Object::disconnect(int connectionId)
{

}
