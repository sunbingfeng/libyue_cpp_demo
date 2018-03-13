// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#ifndef NATIVEUI_WIN_BROWSER_BROWSER_HTML_MONIKER_H_
#define NATIVEUI_WIN_BROWSER_BROWSER_HTML_MONIKER_H_

#include <exdisp.h>
#include <mshtmhst.h>
#include <ole2.h>
#include <wrl.h>

#include "base/macros.h"
#include "base/strings/string16.h"

namespace nu {

class BrowserHTMLMoniker : public IMoniker {
 public:
  BrowserHTMLMoniker();
  ~BrowserHTMLMoniker();

  void LoadHTML(const base::string16& str, const base::string16& base_url);

  // IUnknown
  STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
  STDMETHOD_(ULONG, AddRef)();
  STDMETHOD_(ULONG, Release)();

  // IMoniker
  STDMETHODIMP BindToStorage(IBindCtx *pbc, IMoniker *pmkToLeft, REFIID riid,
                             void **ppvObj);
  STDMETHODIMP GetDisplayName(IBindCtx *pbc, IMoniker *pmkToLeft,
                              LPOLESTR *ppszDisplayName);
  STDMETHODIMP BindToObject(IBindCtx *pbc, IMoniker *pmkToLeft,
                            REFIID riidResult, void **ppvResult);
  STDMETHODIMP Reduce(IBindCtx *pbc, DWORD dwReduceHowFar,
                      IMoniker **ppmkToLeft, IMoniker **ppmkReduced);
  STDMETHODIMP ComposeWith(IMoniker *pmkRight, BOOL fOnlyIfNotGeneric,
                           IMoniker **ppmkComposite);
  STDMETHODIMP Enum(BOOL fForward, IEnumMoniker **ppenumMoniker);
  STDMETHODIMP IsEqual(IMoniker *pmkOtherMoniker);
  STDMETHODIMP Hash(DWORD *pdwHash);
  STDMETHODIMP IsRunning(IBindCtx *pbc, IMoniker *pmkToLeft,
                         IMoniker *pmkNewlyRunning);
  STDMETHODIMP GetTimeOfLastChange(IBindCtx *pbc, IMoniker *pmkToLeft,
                                   FILETIME *pFileTime);
  STDMETHODIMP Inverse(IMoniker **ppmk);
  STDMETHODIMP CommonPrefixWith(IMoniker *pmkOther, IMoniker **ppmkPrefix);
  STDMETHODIMP RelativePathTo(IMoniker *pmkOther, IMoniker **ppmkRelPath);
  STDMETHODIMP ParseDisplayName(IBindCtx *pbc, IMoniker *pmkToLeft,
                                LPOLESTR pszDisplayName, ULONG *pchEaten,
                                IMoniker **ppmkOut);
  STDMETHODIMP IsSystemMoniker(DWORD *pdwMksys);

  // IPersistStream
  STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty);
  STDMETHODIMP IsDirty();
  STDMETHODIMP Load(IStream *pStm);
  STDMETHODIMP GetSizeMax(ULARGE_INTEGER *pcbSize);

  // IPersist
  STDMETHODIMP GetClassID(CLSID *pClassID);

 private:
  ULONG ref_;

  base::string16 base_url_;
  Microsoft::WRL::ComPtr<IStream> stream_;

  DISALLOW_COPY_AND_ASSIGN(BrowserHTMLMoniker);
};

}  // namespace nu

#endif  // NATIVEUI_WIN_BROWSER_BROWSER_HTML_MONIKER_H_
