///

#pragma once

class IPackager
{
public:
	virtual void Init ( long sizeLimit ) = 0 ;
	virtual HRESULT PackageAndSend (	BSTR				endpoint, 
										BSTR				envelope, 
										ISoapAttachments *	parts, 
										ISOAPTransport *	transport ) = 0 ;
	virtual HRESULT Receive        ( /*[in,out]*/		BSTR * characterEncoding, 
									/*[out,retval]*/	SAFEARRAY ** Envelope, 
									/*[in]*/			ISoapAttachments * parts, 
									/*[in]*/			ISOAPTransport * transport ) = 0 ;
};