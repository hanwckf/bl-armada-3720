/*******************************************************************************
Copyright (C) 2017 Marvell International Ltd.

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the three
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation, either version 2 of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************************
Marvell GNU General Public License FreeRTOS Exception

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the Lesser
General Public License Version 2.1 plus the following FreeRTOS exception.
An independent module is a module which is not derived from or based on
FreeRTOS.
Clause 1:
Linking FreeRTOS statically or dynamically with other modules is making a
combined work based on FreeRTOS. Thus, the terms and conditions of the GNU
General Public License cover the whole combination.
As a special exception, the copyright holder of FreeRTOS gives you permission
to link FreeRTOS with independent modules that communicate with FreeRTOS solely
through the FreeRTOS API interface, regardless of the license terms of these
independent modules, and to copy and distribute the resulting combined work
under terms of your choice, provided that:
1. Every copy of the combined work is accompanied by a written statement that
details to the recipient the version of FreeRTOS used and an offer by yourself
to provide the FreeRTOS source code (including any modifications you may have
made) should the recipient request it.
2. The combined work is not itself an RTOS, scheduler, kernel or related
product.
3. The independent modules add significant and primary functionality to
FreeRTOS and do not merely extend the existing functionality already present in
FreeRTOS.
Clause 2:
FreeRTOS may not be used for any competitive or comparative purpose, including
the publication of any form of run time or compile time metric, without the
express permission of Real Time Engineers Ltd. (this is the norm within the
industry and is intended to ensure information accuracy).

********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.

	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.

	* Neither the name of Marvell nor the names of its contributors may be
	  used to endorse or promote products derived from this software without
	  specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/


// This file is used to create a workaround, so that we can use the CryptoPP
// library functions to sign a message using only its hash..
// CryptoPP library does not support this feature currently..
// (tested with CryptoPP library version 5.6.5)

#ifndef __CRYPTOPP_L_INTREFACE_H__
#define __CRYPTOPP_L_INTREFACE_H__


#define INCLUDE_FILE(fname) <LIBDIR/fname>

#include INCLUDE_FILE(hex.h)
#include INCLUDE_FILE(rsa.h)
#include INCLUDE_FILE(eccrypto.h)
#include INCLUDE_FILE(integer.h)
#include INCLUDE_FILE(osrng.h)
#include INCLUDE_FILE(oids.h)
#include INCLUDE_FILE(rsa.h)
#include INCLUDE_FILE(pkcspad.h)
#include INCLUDE_FILE(pssr.h)
#include INCLUDE_FILE(iterhash.h)
#include INCLUDE_FILE(sha.h)
#include INCLUDE_FILE(files.h)
#include INCLUDE_FILE(secblock.h)


namespace CryptoPP {

// Create a new Signer abstract base class, which has
// function to sign a hash
class PK_Signer_Hash : public PK_Signer
{
public:

	/***/	size_t SignHash(RandomNumberGenerator &rng, const byte *hashMessage, byte *signature) const;
	virtual size_t SignHashAndRestart(RandomNumberGenerator &rng, PK_MessageAccumulator &messageAccumulator, byte* hash, byte *signature, bool restart = true) const = 0;
};


/*******************************************************************************************************
 * The following classes are required to implement the
 * RSA functions which can be used to sign a message
 * using only its Hash.. The following schemes are supported:
 * 1. PKCS1v15		(PKCS v 1.5)
 * 2. PSS 			(PKCS v 2.2)
 *******************************************************************************************************/


// RSA Signer base class.. Implements the PK_Signer_Hash interface (or Abstract Class)
// instead of PK_Signer
class TF_SignerBase_Hash : public TF_SignatureSchemeBase<PK_Signer_Hash, TF_Base<RandomizedTrapdoorFunctionInverse, PK_SignatureMessageEncodingMethod> >
{
public:
	void InputRecoverableMessage(PK_MessageAccumulator &messageAccumulator, const byte *recoverableMessage, size_t recoverableMessageLength) const
	{	throw NotImplemented("CryptoPP_L_interface: Use native CryptoPP classes for this functionality");}
	virtual size_t SignAndRestart(RandomNumberGenerator &rng, PK_MessageAccumulator &messageAccumulator, byte *signature, bool restart=true) const
	{	throw NotImplemented("CryptoPP_L_interface: Use native CryptoPP classes for this functionality");}

	size_t SignHashAndRestart(RandomNumberGenerator &rng, PK_MessageAccumulator &messageAccumulator,byte* hash, byte *signature, bool restart = true) const;
};

// RSA Signer intermediate class.. Inherits the TF_SignerBase_Hash class instead of TF_SignerBase
// (For Hash signing support)
template <class SCHEME_OPTIONS>
class TF_SignerImpl_Hash : public TF_ObjectImpl<TF_SignerBase_Hash, SCHEME_OPTIONS, typename SCHEME_OPTIONS::PrivateKey>
{
};

// Struct RSASS_Hash inherits this class..
// This class contains the modified signer instance
template <class STANDARD, class H, class KEYS, class ALG_INFO = TF_SS<STANDARD, H, KEYS, int> >	// VC60 workaround: doesn't work if KEYS is first parameter
class TF_SS_Hash : public KEYS
{
public:
	//! see SignatureStandard for a list of standards
	typedef STANDARD Standard;
	typedef typename Standard::SignatureMessageEncodingMethod MessageEncodingMethod;
	typedef TF_SignatureSchemeOptions<ALG_INFO, KEYS, MessageEncodingMethod, H> SchemeOptions;

	static std::string CRYPTOPP_API StaticAlgorithmName() {return std::string(KEYS::StaticAlgorithmName()) + "/" + MessageEncodingMethod::StaticAlgorithmName() + "(" + H::StaticAlgorithmName() + ")";}

	//! implements PK_Signer interface
	typedef PK_FinalTemplate<TF_SignerImpl_Hash<SchemeOptions> > SignerHash;
	//! implements PK_Verifier interface
	typedef PK_FinalTemplate<TF_VerifierImpl<SchemeOptions> > Verifier;
};

// We inherit the classes where the message representative is computed
// for RSA and redefine the function to compute it

//For PKCS1v15
class PKCS1v15_SignatureMessageEncodingMethod_Hash : public PKCS1v15_SignatureMessageEncodingMethod
{
	void ComputeMessageRepresentative(RandomNumberGenerator &rng,
			const byte *recoverableMessage, size_t recoverableMessageLength,
			HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
			byte *representative, size_t representativeBitLength) const;
};

//For PSS
template <bool ALLOW_RECOVERY, class MGF=P1363_MGF1, int SALT_LEN=-1, int MIN_PAD_LEN=0, bool USE_HASH_ID=false>
class PSSR_MEM_Hash : public PSSR_MEM_BaseWithHashId<false>
{
	bool AllowRecovery() const {return ALLOW_RECOVERY;}
	size_t SaltLen(size_t hashLen) const {return SALT_LEN < 0 ? hashLen : SALT_LEN;}
	size_t MinPadLen(size_t hashLen) const {return MIN_PAD_LEN < 0 ? hashLen : MIN_PAD_LEN;}
	const MaskGeneratingFunction & GetMGF() const {static MGF mgf; return mgf;}

public:
	static std::string CRYPTOPP_API StaticAlgorithmName() {return std::string(ALLOW_RECOVERY ? "PSSR-" : "PSS-") + MGF::StaticAlgorithmName();}

	void ComputeMessageRepresentative(RandomNumberGenerator &rng,
			const byte *recoverableMessage, size_t recoverableMessageLength,
			HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
			byte *representative, size_t representativeBitLength) const;

};

// We will need new typedefs for PKCS1v15 and PSS
// which will support hash signing
struct PKCS1v15_Hash : public SignatureStandard, public EncryptionStandard
{
	typedef PKCS_EncryptionPaddingScheme EncryptionMessageEncodingMethod;
	typedef PKCS1v15_SignatureMessageEncodingMethod_Hash SignatureMessageEncodingMethod;
};

struct PSS_Hash : public SignatureStandard
{
	typedef PSSR_MEM_Hash<false> SignatureMessageEncodingMethod;
};

// The modified struct RSASS, the caller needs to instantiate
template <class STANDARD, class H>
struct RSASS_Hash : public TF_SS_Hash<STANDARD, H, RSA>
{
};

/*******************************************************************************************************
 * The following classes are required to implement the
 * ECDSA functions which can be used to sign a message
 * using only its Hash..
 *******************************************************************************************************/


// ECDSA Signer base class.. Implements the PK_Signer_Hash interface (or Abstract Class)
// instead of PK_Signer
template <class T>
class CRYPTOPP_NO_VTABLE DL_SignerBase_Hash : public DL_SignatureSchemeBase<PK_Signer_Hash, DL_PrivateKey<T> >
{
public:
	virtual ~DL_SignerBase_Hash() { }

	void RawSign(const Integer &k, const Integer &e, Integer &r, Integer &s) const
	{	throw NotImplemented("CryptoPP_L_interface: Use native CryptoPP classes for this functionality");}
	void InputRecoverableMessage(PK_MessageAccumulator &messageAccumulator, const byte *recoverableMessage, size_t recoverableMessageLength) const
	{	throw NotImplemented("CryptoPP_L_interface: Use native CryptoPP classes for this functionality");}
	size_t SignAndRestart(RandomNumberGenerator &rng, PK_MessageAccumulator &messageAccumulator, byte *signature, bool restart) const
	{	throw NotImplemented("CryptoPP_L_interface: Use native CryptoPP classes for this functionality");}

	// Function to sign the Hash...
	// Template functions need to be defined in the header
	size_t SignHashAndRestart(RandomNumberGenerator &rng, PK_MessageAccumulator &messageAccumulator,byte* hash, byte *signature, bool restart) const
	{
		//return SignAndRestart(rng, messageAccumulator, signature, restart);
		this->GetMaterial().DoQuickSanityCheck();
		PK_MessageAccumulatorBase &ma = static_cast<PK_MessageAccumulatorBase &>(messageAccumulator);

		const DL_ElgamalLikeSignatureAlgorithm<T> &alg = this->GetSignatureAlgorithm();
		const DL_GroupParameters<T> &params = this->GetAbstractGroupParameters();
		const DL_PrivateKey<T> &key = this->GetKeyInterface();

		if (ma.m_recoverableMessage.size() > 0)
			throw NotImplemented("CryptoPP_L_interface: Use native CryptoPP classes for recoverable message size > 0");

		SecByteBlock representative(this->MessageRepresentativeLength());
		this->GetMessageEncodingInterface().ComputeMessageRepresentative(
				rng,hash, ma.m_recoverableMessage.size(),
				ma.AccessHash(), this->GetHashIdentifier(), ma.m_empty,
				representative, this->MessageRepresentativeBitLength());
		ma.m_empty = true;
		Integer e(representative, representative.size());

		// hash message digest into random number k to prevent reusing the same k on a different messages
		// after virtual machine rollback
		if (rng.CanIncorporateEntropy())
			rng.IncorporateEntropy(representative, representative.size());
		Integer k(rng, 1, params.GetSubgroupOrder() - 1);
		Integer r, s;
		r = params.ConvertElementToInteger(params.ExponentiateBase(k));
		alg.Sign(params, key.GetPrivateExponent(), k, e, r, s);

		size_t rLen = alg.RLen(params);
		r.Encode(signature, rLen);
		s.Encode(signature + rLen, alg.SLen(params));

		if (restart)
			RestartMessageAccumulator(rng, ma);

		return this->SignatureLength();
	}

protected:
	void RestartMessageAccumulator(RandomNumberGenerator &rng, PK_MessageAccumulatorBase &ma) const	{}

};

// ECDSA Signer intermediate class.. Inherits the DL_SignerBase_Hash class instead of DL_SignerBase
// (For Hash signing support)
template <class SCHEME_OPTIONS>
class DL_SignerImpl_Hash : public DL_ObjectImpl<DL_SignerBase_Hash<typename SCHEME_OPTIONS::Element>, SCHEME_OPTIONS, typename SCHEME_OPTIONS::PrivateKey>
{
public:
	PK_MessageAccumulator * NewSignatureAccumulator(RandomNumberGenerator &rng) const
	{
		member_ptr<PK_MessageAccumulatorBase> p(new PK_MessageAccumulatorImpl<CPP_TYPENAME SCHEME_OPTIONS::HashFunction>);
		this->RestartMessageAccumulator(rng, *p);
		return p.release();
	}
};

// Struct ECDSA_Hash inherits this class..
// This class contains the modified signer instance
template <class KEYS, class SA, class MEM, class H, class ALG_INFO = DL_SS<KEYS, SA, MEM, H, int> >
class DL_SS_Hash : public KEYS
{
	typedef DL_SignatureSchemeOptions<ALG_INFO, KEYS, SA, MEM, H> SchemeOptions;

public:
	static std::string StaticAlgorithmName() {return SA::StaticAlgorithmName() + std::string("/EMSA1(") + H::StaticAlgorithmName() + ")";}

	//! implements PK_Signer interface
	typedef PK_FinalTemplate<DL_SignerImpl_Hash<SchemeOptions> > SignerHash;
	//! implements PK_Verifier interface
	typedef PK_FinalTemplate<DL_VerifierImpl<SchemeOptions> > Verifier;
};


// We inherit the class where the message representative is computed
// for ECDSA and redefine the function to compute it
class DL_SignatureMessageEncodingMethod_DSA_Hash : public DL_SignatureMessageEncodingMethod_DSA
{
public:
	void ComputeMessageRepresentative(RandomNumberGenerator &rng,
		const byte *recoverableMessage, size_t recoverableMessageLength,
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, size_t representativeBitLength) const;
};

// The modified struct ECDSA, the caller needs to instantiate
template <class EC, class H>
struct ECDSA_Hash : public DL_SS_Hash<DL_Keys_ECDSA<EC>, DL_Algorithm_ECDSA<EC>, DL_SignatureMessageEncodingMethod_DSA_Hash, H>
{
	virtual ~ECDSA_Hash() {}
};

}

#endif // __CRYPTOPP_L_INTREFACE_H__
