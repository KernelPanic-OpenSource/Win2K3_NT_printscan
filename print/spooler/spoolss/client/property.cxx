/*++

Copyright (c) 1996  Microsoft Corporation

Abstract:

    Adds properties to ds

Author:

    Steve Wilson (NT) December 1996

Revision History:

--*/

#define INC_OLE2

#include "precomp.h"
#pragma hdrstop

#include "varconv.hxx"
#include "property.hxx"
#include "client.h"


#define VALIDATE_PTR(pPtr) \
    if (!pPtr) { \
        hr = E_ADS_BAD_PARAMETER;\
    }\
    BAIL_ON_FAILURE(hr);




HRESULT
put_BSTR_Property(
    IADs * pADsObject,
    BSTR   bstrPropertyName,
    BSTR   pSrcStringProperty
    )
{
    HRESULT hr;
    VARIANT varInputData;

    hr = PackString2Variant(
            pSrcStringProperty,
            &varInputData
            );
    BAIL_ON_FAILURE(hr);

    if (!pSrcStringProperty || !*pSrcStringProperty) {
        hr = pADsObject->PutEx(
                ADS_PROPERTY_CLEAR,
                bstrPropertyName,
                varInputData
                );
        BAIL_ON_FAILURE(hr);

    } else {

        hr = pADsObject->Put(
                bstrPropertyName,
                varInputData
                );
        BAIL_ON_FAILURE(hr);
    }

error:

    VariantClear(&varInputData);

    return hr;
}


HRESULT
put_DWORD_Property(
    IADs  *pADsObject,
    BSTR   bstrPropertyName,
    DWORD *pdwSrcProperty
    )
{
    HRESULT hr = S_OK;
    VARIANT varInputData;

    if (!pdwSrcProperty)
        return S_OK;

    hr = PackDWORD2Variant(
            *pdwSrcProperty,
            &varInputData
            );
    BAIL_ON_FAILURE(hr);


    hr = pADsObject->Put(
            bstrPropertyName,
            varInputData
            );
    BAIL_ON_FAILURE(hr);

error:
    return hr;
}

HRESULT
put_MULTISZ_Property(
    IADs    *pADsObject,
    BSTR    bstrPropertyName,
    BSTR    pSrcStringProperty
    )
{
    HRESULT hr = S_OK;
    VARIANT var;
    VARIANT varInputData;
    BSTR    pStr;
    BSTR    *pStrArray;
    DWORD   i;
    BSTR    pMultiString;

    if (!pSrcStringProperty || !*pSrcStringProperty)
        pMultiString = L"";
    else
        pMultiString = pSrcStringProperty;

    VariantInit(&var);
    //
    // Convert MULTI_SZ to string array (last element of array must be NULL)
    //
    for (i = 0, pStr = pMultiString ; *pStr ; ++i, pStr += wcslen(pStr) + 1)
        ;

    if (!(pStrArray = (BSTR *) AllocSplMem((i + 1)*sizeof(BSTR)))) {
        hr = MAKE_HRESULT( SEVERITY_ERROR, FACILITY_WIN32, GetLastError());
        BAIL_ON_FAILURE(hr);
    }

    for (i = 0, pStr = pMultiString ; *pStr ; ++i, pStr += wcslen(pStr) + 1)
        pStrArray[i] = pStr;
    pStrArray[i] = NULL;

    MakeVariantFromStringArray(pStrArray, &var);

    FreeSplMem(pStrArray);

    hr = PackVARIANTinVariant(
            var,
            &varInputData
            );
    BAIL_ON_FAILURE(hr);

    if (!pSrcStringProperty || !*pSrcStringProperty) {
        hr = pADsObject->PutEx(
                ADS_PROPERTY_CLEAR,
                bstrPropertyName,
                varInputData
                );
        BAIL_ON_FAILURE(hr);

    } else {

        hr = pADsObject->Put(
                bstrPropertyName,
                varInputData
                );
        BAIL_ON_FAILURE(hr);
    }

error:

    VariantClear(&var);
    VariantClear(&varInputData);

    return hr;
}

HRESULT
put_BOOL_Property(
    IADs *pADsObject,
    BSTR bstrPropertyName,
    BOOL *bSrcProperty
    )
{
    HRESULT hr = S_OK;
    VARIANT varInputData;
    BOOL    bVal;

    bVal = bSrcProperty ? *bSrcProperty : 0;

    hr = PackBOOL2Variant(
            bVal,
            &varInputData
            );
    BAIL_ON_FAILURE(hr);

    if (!bSrcProperty) {
        hr = pADsObject->PutEx(
                ADS_PROPERTY_CLEAR,
                bstrPropertyName,
                varInputData
                );
        BAIL_ON_FAILURE(hr);

    } else {

        hr = pADsObject->Put(
                bstrPropertyName,
                varInputData
                );
        BAIL_ON_FAILURE(hr);
    }

error:
    return hr;
}

HRESULT
get_BSTR_Property(
    IADs *pADsObject,
    BSTR  bstrPropertyName,
    BSTR *ppDestStringProperty
    )
{
    HRESULT hr = S_OK;
    VARIANT varOutputData;

    VariantInit( &varOutputData );

    hr = pADsObject->Get(
            bstrPropertyName,
            &varOutputData
            );
    BAIL_ON_FAILURE(hr);

    hr = UnpackStringfromVariant(
            varOutputData,
            ppDestStringProperty
            );
    BAIL_ON_FAILURE(hr);


error:
    return hr;
}





