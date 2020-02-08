// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "UiaTextRange.hpp"
#include "TermControlUiaProvider.hpp"

using namespace Microsoft::Terminal;
using namespace Microsoft::Console::Types;
using namespace Microsoft::WRL;

// degenerate range constructor.
HRESULT UiaTextRange::RuntimeClassInitialize(_In_ IUiaData* pData, _In_ IRawElementProviderSimple* const pProvider, _In_ const std::wstring_view wordDelimiters)
{
    return UiaTextRangeBase::RuntimeClassInitialize(pData, pProvider, wordDelimiters);
}

HRESULT UiaTextRange::RuntimeClassInitialize(_In_ IUiaData* pData,
                                             _In_ IRawElementProviderSimple* const pProvider,
                                             const Cursor& cursor,
                                             const std::wstring_view wordDelimiters)
{
    return UiaTextRangeBase::RuntimeClassInitialize(pData, pProvider, cursor, wordDelimiters);
}

HRESULT UiaTextRange::RuntimeClassInitialize(_In_ IUiaData* pData,
                                             _In_ IRawElementProviderSimple* const pProvider,
                                             const COORD start,
                                             const COORD end,
                                             bool blockSelection,
                                             const std::wstring_view wordDelimiters)
{
    return UiaTextRangeBase::RuntimeClassInitialize(pData, pProvider, start, end, blockSelection, wordDelimiters);
}

// returns a degenerate text range of the start of the row closest to the y value of point
HRESULT UiaTextRange::RuntimeClassInitialize(_In_ IUiaData* pData,
                                             _In_ IRawElementProviderSimple* const pProvider,
                                             const UiaPoint point,
                                             const std::wstring_view wordDelimiters)
{
    RETURN_IF_FAILED(UiaTextRangeBase::RuntimeClassInitialize(pData, pProvider, wordDelimiters));
    Initialize(point);
    return S_OK;
}

HRESULT UiaTextRange::RuntimeClassInitialize(const UiaTextRange& a)
{
    return UiaTextRangeBase::RuntimeClassInitialize(a);
}

IFACEMETHODIMP UiaTextRange::Clone(_Outptr_result_maybenull_ ITextRangeProvider** ppRetVal)
{
    RETURN_HR_IF(E_INVALIDARG, ppRetVal == nullptr);
    *ppRetVal = nullptr;
    auto hr = MakeAndInitialize<UiaTextRange>(ppRetVal, *this);

    if (hr != S_OK)
    {
        *ppRetVal = nullptr;
        return hr;
    }

#if defined(_DEBUG) && defined(UiaTextRangeBase_DEBUG_MSGS)
    OutputDebugString(L"Clone\n");
    std::wstringstream ss;
    ss << _id << L" cloned to " << (static_cast<UiaTextRangeBase*>(*ppRetVal))->_id;
    std::wstring str = ss.str();
    OutputDebugString(str.c_str());
    OutputDebugString(L"\n");
#endif
    // TODO GitHub #1914: Re-attach Tracing to UIA Tree
    // tracing
    /*ApiMsgClone apiMsg;
    apiMsg.CloneId = static_cast<UiaTextRangeBase*>(*ppRetVal)->GetId();
    Tracing::s_TraceUia(this, ApiCall::Clone, &apiMsg);*/

    return S_OK;
}

void UiaTextRange::_ChangeViewport(const SMALL_RECT /*NewWindow*/)
{
    // TODO GitHub #2361: Update viewport when calling UiaTextRangeBase::ScrollIntoView()
}

// Method Description:
// - Transform coordinates relative to the client to relative to the screen
// Arguments:
// - clientPoint: coordinates relative to the client where
//                (0,0) is the top-left of the app window
// Return Value:
// - <none>
void UiaTextRange::_TranslatePointToScreen(LPPOINT clientPoint) const
{
    auto provider = static_cast<TermControlUiaProvider*>(_pProvider);

    // update based on TermControl location (important for Panes)
    UiaRect boundingRect;
    THROW_IF_FAILED(provider->get_BoundingRectangle(&boundingRect));
    clientPoint->x += gsl::narrow<LONG>(boundingRect.left);
    clientPoint->y += gsl::narrow<LONG>(boundingRect.top);

    // update based on TermControl padding
    auto padding = provider->GetPadding();
    clientPoint->x += gsl::narrow<LONG>(padding.Left);
    clientPoint->y += gsl::narrow<LONG>(padding.Top);
}

void UiaTextRange::_TranslatePointFromScreen(LPPOINT /*screenPoint*/) const
{
    // TODO GitHub #2103: NON-HWND IMPLEMENTATION OF SCREENTOCLIENT()
}

const COORD UiaTextRange::_getScreenFontSize() const
{
    // Do NOT get the font info from IRenderData. It is a dummy font info.
    // Instead, the font info is saved in the TermControl. So we have to
    // ask our parent to get it for us.
    auto provider = static_cast<TermControlUiaProvider*>(_pProvider);
    return provider->GetFontSize();
}
