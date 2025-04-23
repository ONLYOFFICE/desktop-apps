#ifndef SHELLASSOC_H
#define SHELLASSOC_H

struct AssocPair {
    const wchar_t* extension;
    const wchar_t* progId;
};

bool SetUserFileAssoc(const wchar_t* ext, const wchar_t* progId, bool notifySystem = true);
bool SetUserFileAssoc(const AssocPair *assocArray, size_t count);
// bool ResetUserFileAssoc(const wchar_t* ext, bool notifySystem = true);
// bool ResetUserFileAssoc(const wchar_t** extArray, size_t count);

#endif // SHELLASSOC_H
