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



#include "CryptoPP_L_interface.h"
namespace CryptoPP {

size_t PK_Signer_Hash::SignHash(CryptoPP::RandomNumberGenerator &rng, const byte *hashMessage, byte *signature) const
{
	std::auto_ptr<PK_MessageAccumulator> m(NewSignatureAccumulator(rng));
	return SignHashAndRestart(rng, *m,(byte *) hashMessage, signature, false);
}

size_t TF_SignerBase_Hash::SignHashAndRestart(RandomNumberGenerator &rng, PK_MessageAccumulator &messageAccumulator,byte* hash, byte *signature, bool restart) const
{
	PK_MessageAccumulatorBase &ma = static_cast<PK_MessageAccumulatorBase &>(messageAccumulator);
	HashIdentifier id = GetHashIdentifier();
	const MessageEncodingInterface &encoding = GetMessageEncodingInterface();

	if (MessageRepresentativeBitLength() < encoding.MinRepresentativeBitLength(id.second, ma.AccessHash().DigestSize()))
		throw PK_SignatureScheme::KeyTooShort();

	SecByteBlock representative(MessageRepresentativeLength());
	if (ma.m_recoverableMessage.size() > 0)
		throw NotImplemented("CryptoPP_L_interface: Use native CryptoPP classes for recoverable message size > 0");
	
	encoding.ComputeMessageRepresentative(rng,
			hash, ma.m_recoverableMessage.size(),
			ma.AccessHash(), id, ma.m_empty,
			representative, MessageRepresentativeBitLength());
	ma.m_empty = true;

	Integer r(representative, representative.size());
	size_t signatureLength = SignatureLength();
	GetTrapdoorFunctionInterface().CalculateRandomizedInverse(rng, r).Encode(signature, signatureLength);
	return signatureLength;
}

// Function computing message representative for PKCS1v15
void PKCS1v15_SignatureMessageEncodingMethod_Hash::ComputeMessageRepresentative(RandomNumberGenerator &rng,
		const byte *hashMsg, size_t recoverableMessageLength,
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, size_t representativeBitLength) const
{

	size_t pkcsBlockLen = representativeBitLength;
	// convert from bit length to byte length
	if (pkcsBlockLen % 8 != 0)
	{
		representative[0] = 0;
		representative++;
	}
	pkcsBlockLen /= 8;

	representative[0] = 1;   // block type 1

	unsigned int digestSize = hash.DigestSize();
	byte *pPadding = representative + 1;
	byte *pDigest = representative + pkcsBlockLen - digestSize;
	byte *pHashId = pDigest - hashIdentifier.second;
	byte *pSeparator = pHashId - 1;

	// pad with 0xff
	memset(pPadding, 0xff, pSeparator - pPadding);
	*pSeparator = 0;
	memcpy(pHashId, hashIdentifier.first, hashIdentifier.second);
	memcpy(pDigest,hashMsg,digestSize);
}

// Function computing message representative for PKCSv22
template<>
void PSSR_MEM_Hash<false>::ComputeMessageRepresentative(RandomNumberGenerator &rng,
		const byte *hashMsg, size_t recoverableMessageLength,
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, size_t representativeBitLength) const
{

	const size_t u = hashIdentifier.second + 1;
	const size_t representativeByteLength = BitsToBytes(representativeBitLength);
	const size_t digestSize = hash.DigestSize();
	const size_t saltSize = SaltLen(digestSize);
	byte *const h = representative + representativeByteLength - u - digestSize;

	SecByteBlock digest(digestSize), salt(saltSize);

	digest.Assign(hashMsg, digestSize);
	rng.GenerateBlock(salt, saltSize);
	// compute H = hash of M'
	byte c[8];
	PutWord(false, BIG_ENDIAN_ORDER, c, (word32)SafeRightShift<29>(recoverableMessageLength));
	PutWord(false, BIG_ENDIAN_ORDER, c + 4, word32(recoverableMessageLength << 3));
	hash.Update(c, 8);
	hash.Update(digest, digestSize);
	hash.Update(salt, saltSize);
	hash.Final(h);

	// compute representative
	GetMGF().GenerateAndMask(hash, representative, representativeByteLength - u - digestSize, h, digestSize, false);
	byte *xorStart = representative + representativeByteLength - u - digestSize - salt.size() - recoverableMessageLength - 1;
	xorStart[0] ^= 1;
	xorbuf(xorStart + 1 + recoverableMessageLength, salt, salt.size());
	memcpy(representative + representativeByteLength - u, hashIdentifier.first, hashIdentifier.second);
	representative[representativeByteLength - 1] = hashIdentifier.second ? 0xcc : 0xbc;
	if (representativeBitLength % 8 != 0)
		representative[0] = (byte)Crop(representative[0], representativeBitLength % 8);
}
	


// Function comuting message representative for ECDSA	
void DL_SignatureMessageEncodingMethod_DSA_Hash::ComputeMessageRepresentative(RandomNumberGenerator &rng,
		const byte *hashMsg, size_t recoverableMessageLength,
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, size_t representativeBitLength) const
{
	const size_t representativeByteLength = BitsToBytes(representativeBitLength);
	const size_t digestSize = hash.DigestSize();
	const size_t paddingLength = SaturatingSubtract(representativeByteLength, digestSize);

	memset(representative, 0, paddingLength);
	memcpy(representative + paddingLength, hashMsg, digestSize);
	if (digestSize * 8 > representativeBitLength)
	{
		Integer h(representative, representativeByteLength);
		h >>= representativeByteLength * 8 - representativeBitLength;
		h.Encode(representative, representativeByteLength);
	}
}

}

