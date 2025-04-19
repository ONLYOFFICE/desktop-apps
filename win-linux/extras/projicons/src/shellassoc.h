#ifndef SHELLASSOC_H
#define SHELLASSOC_H

struct AssocPair {
    const wchar_t* extension;
    const wchar_t* progId;
};

bool SetUserFileAssoc(const wchar_t* ext, const wchar_t* progId);
bool SetUserFileAssoc(const AssocPair *assocArray, size_t count);
bool ClearUserFileAssoc(const wchar_t* ext);

#endif // SHELLASSOC_H
