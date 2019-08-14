/*
 * ZeroTier One - Network Virtualization Everywhere
 * Copyright (C) 2011-2019  ZeroTier, Inc.  https://www.zerotier.com/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * --
 *
 * You can be released from the requirements of the license by purchasing
 * a commercial license. Buying such a license is mandatory as soon as you
 * develop commercial closed-source software that incorporates or links
 * directly against ZeroTier software without disclosing the source code
 * of your own application.
 */

#include "Constants.hpp"
#include "RuntimeEnvironment.hpp"
#include "Credential.hpp"
#include "Capability.hpp"
#include "Tag.hpp"
#include "CertificateOfMembership.hpp"
#include "CertificateOfOwnership.hpp"
#include "Revocation.hpp"
#include "Switch.hpp"
#include "Network.hpp"

namespace ZeroTier {

template<typename CRED>
static inline Credential::VerifyResult _credVerify(const RuntimeEnvironment *const RR,void *tPtr,CRED credential)
{
	const Address signedBy(credential.signer());
	const uint64_t networkId = credential.networkId();
	if ((!signedBy)||(signedBy != Network::controllerFor(networkId)))
		return Credential::VERIFY_BAD_SIGNATURE;
	const Identity id(RR->topology->getIdentity(tPtr,signedBy));
	if (!id) {
		RR->sw->requestWhois(tPtr,RR->node->now(),signedBy);
		return Credential::VERIFY_NEED_IDENTITY;
	}
	try {
		Buffer<(sizeof(CRED) + 64)> *const tmp = new Buffer<(sizeof(CRED) + 64)>();
		credential.serialize(*tmp,true);
		const Credential::VerifyResult result = (id.verify(tmp->data(),tmp->size(),credential.signature(),credential.signatureLength()) ? Credential::VERIFY_OK : Credential::VERIFY_BAD_SIGNATURE);
		delete tmp;
		return result;
	} catch ( ... ) {}
	return Credential::VERIFY_BAD_SIGNATURE;
}

Credential::VerifyResult Credential::_verify(const RuntimeEnvironment *const RR,void *tPtr,const Revocation &credential) const { return _credVerify(RR,tPtr,credential); }
Credential::VerifyResult Credential::_verify(const RuntimeEnvironment *const RR,void *tPtr,const Tag &credential) const { return _credVerify(RR,tPtr,credential); }
Credential::VerifyResult Credential::_verify(const RuntimeEnvironment *const RR,void *tPtr,const CertificateOfOwnership &credential) const { return _credVerify(RR,tPtr,credential); }

Credential::VerifyResult Credential::_verify(const RuntimeEnvironment *const RR,void *tPtr,const CertificateOfMembership &credential) const
{
	if ((!credential._signedBy)||(credential._signedBy != Network::controllerFor(credential.networkId()))||(credential._qualifierCount > ZT_NETWORK_COM_MAX_QUALIFIERS))
		return Credential::VERIFY_BAD_SIGNATURE;

	const Identity id(RR->topology->getIdentity(tPtr,credential._signedBy));
	if (!id) {
		RR->sw->requestWhois(tPtr,RR->node->now(),credential._signedBy);
		return Credential::VERIFY_NEED_IDENTITY;
	}

	uint64_t buf[ZT_NETWORK_COM_MAX_QUALIFIERS * 3];
	unsigned int ptr = 0;
	for(unsigned int i=0;i<credential._qualifierCount;++i) {
		buf[ptr++] = Utils::hton(credential._qualifiers[i].id);
		buf[ptr++] = Utils::hton(credential._qualifiers[i].value);
		buf[ptr++] = Utils::hton(credential._qualifiers[i].maxDelta);
	}

	return (id.verify(buf,ptr * sizeof(uint64_t),credential._signature,credential._signatureLength) ? Credential::VERIFY_OK : Credential::VERIFY_BAD_SIGNATURE);
}

Credential::VerifyResult Credential::_verify(const RuntimeEnvironment *RR,void *tPtr,const Capability &credential) const
{
	try {
		// There must be at least one entry, and sanity check for bad chain max length
		if ((credential._maxCustodyChainLength < 1)||(credential._maxCustodyChainLength > ZT_MAX_CAPABILITY_CUSTODY_CHAIN_LENGTH))
			return Credential::VERIFY_BAD_SIGNATURE;

		// Validate all entries in chain of custody
		Buffer<(sizeof(Capability) * 2)> tmp;
		credential.serialize(tmp,true);
		for(unsigned int c=0;c<credential._maxCustodyChainLength;++c) {
			if (c == 0) {
				if ((!credential._custody[c].to)||(!credential._custody[c].from)||(credential._custody[c].from != Network::controllerFor(credential._nwid)))
					return Credential::VERIFY_BAD_SIGNATURE; // the first entry must be present and from the network's controller
			} else {
				if (!credential._custody[c].to)
					return Credential::VERIFY_OK; // all previous entries were valid, so we are valid
				else if ((!credential._custody[c].from)||(credential._custody[c].from != credential._custody[c-1].to))
					return Credential::VERIFY_BAD_SIGNATURE; // otherwise if we have another entry it must be from the previous holder in the chain
			}

			const Identity id(RR->topology->getIdentity(tPtr,credential._custody[c].from));
			if (id) {
				if (!id.verify(tmp.data(),tmp.size(),credential._custody[c].signature,credential._custody[c].signatureLength))
					return Credential::VERIFY_BAD_SIGNATURE;
			} else {
				RR->sw->requestWhois(tPtr,RR->node->now(),credential._custody[c].from);
				return Credential::VERIFY_NEED_IDENTITY;
			}
		}

		// We reached max custody chain length and everything was valid
		return Credential::VERIFY_OK;
	} catch ( ... ) {}
	return Credential::VERIFY_BAD_SIGNATURE;
}

} // namespace ZeroTier
