/*-
 * Copyright 2005 Colin Percival
 * Copyright 2013 Christian Mehlis & René Kijewski
 * Copyright 2016 Martin Landsmann <martin.landsmann@haw-hamburg.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/lib/libmd/sha256.h,v 1.1.2.1 2005/06/24 13:32:25 cperciva Exp $
 */


/**
 * @ingroup     sys_hashes
 * @{
 *
 * @file
 * @brief       Header definitions for the SHA256 hash function
 *
 * @author      Colin Percival
 * @author      Christian Mehlis
 * @author      Rene Kijewski
 */

#ifndef _SHA256_H_
#define _SHA256_H_

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHA256_DIGEST_LENGTH 32

/**
 * @brief 512 Bit (64 Byte) internally used block size for sha256
 */
#define SHA256_INTERNAL_BLOCK_SIZE (64)

/**
 * @brief Context for ciper operatins based on sha256
 */
typedef struct {
    /** global state */
    uint32_t state[8];
    /** processed bytes counter */
    uint32_t count[2];
    /** data buffer */
    unsigned char buf[64];
} sha256_context_t;

/**
 * @brief sha256-chain indexed element
 */
typedef struct {
    /** the position of this element in its chain */
    size_t index;
    /** the element */
    unsigned char element[SHA256_DIGEST_LENGTH];
} sha256_chain_idx_elm_t;

/**
 * @brief SHA-256 initialization.  Begins a SHA-256 operation.
 *
 * @param ctx  sha256_context_t handle to init
 */
void sha256_init(sha256_context_t *ctx);

/**
 * @brief Add bytes into the hash
 *
 * @param ctx  sha256_context_t handle to use
 * @param[in] data      Input data
 * @param[in] len       Length of @p data
 */
void sha256_update(sha256_context_t *ctx, const uint8_t *data, size_t len);

/**
 * @brief SHA-256 finalization.  Pads the input data, exports the hash value,
 * and clears the context state.
 *
 * @param digest resulting digest, this is the hash of all the bytes
 * @param ctx    sha256_context_t handle to use
 */
void sha256_final(sha256_context_t *ctx, uint8_t *dst);

/**
 * @brief A wrapper function to simplify the generation of a hash, this is
 * usefull for generating sha256 for one buffer
 *
 * @param d pointer to the buffer to generate hash from
 * @param n length of the buffer
 * @param md optional pointer to an array for the result, length must be
 *           SHA256_DIGEST_LENGTH
 *           if md == NULL, one static buffer is used
 */
unsigned char *sha256(const unsigned char *d, size_t n, unsigned char *md);

/**
 * @brief function to compute a hmac-sha256 from a given message
 *
 * @param[in] key key used in the hmac-sha256 computation
 * @param[in] key_length the size in bytes of the key
 * @param[in] message pointer to the message to generate the hmac-sha256
 * @param[in] message_length the length of the message in bytes
 * @param[out] result the computed hmac-sha256,
 *             length MUST be SHA256_DIGEST_LENGTH
 *             if result == NULL, a static buffer is used
 * @returns pointer to the resulting digest.
 *          if result == NULL the pointer points to the static buffer
 */
const unsigned char *hmac_sha256(const unsigned char *key,
                                 size_t key_length,
                                 const unsigned *message,
                                 size_t message_length,
                                 unsigned char *result);

/**
 * @brief function to produce a hash chain statring with a given seed element.
 *        The chain is computed by taking the sha256 from the seed,
 *        hash the resulting sha256 and continuing taking sha256
 *        from each result consecutively.
 *
 * @param[in] seed the seed of the sha256-chain, i.e. the first element
 * @param[in] seed_length the size of seed in bytes
 * @param[in] elements the number of chained elements,
 *            i.e. the index of the last element is (elements-1)
 * @param[out] tail_element the final element of the sha256-chain
 *
 * @returns pointer to tail_element
 */
unsigned char *sha256_chain(const unsigned char *seed, size_t seed_length,
                            size_t elements, unsigned char *tail_element);

/**
 * @brief function to produce a hash chain statring with a given seed element.
 *        The chain is computed the same way as done with sha256_chain().
 *        Additionally intermediate elements are saved while computing the chain.
 *        This slows down computation, but provides the caller with indexed
 *        "waypoint"-elements.
 *        They are supposed to shortcut computing verification elements,
 *        this comes in handy when using long chains,
 *        e.g. a chain with 2<sup>32</sup> elements.
 *
 * @param[in] seed the seed of the sha256-chain, i.e. the first element
 * @param[in] seed_length the size of seed in bytes
 * @param[in] elements the number of chained elements,
 *            i.e. the index of the last element is (elements-1)
 * @param[out] tail_element the final element of the sha256-chain
 * @param[out] waypoints intermediate elements are stored there.
 * @param[in, out] waypoints_length the size of the waypoints array.
 *                 If the given size is equal or greater elements, the complete
 *                 chain will be stored.
 *                 Otherwise every n-th element is stored where:
 *                 n = floor(elements / waypoints_length);
 *                 floor is implicitly used since we perform unsigned integer division.
 *                 The last used waypoint index is stored in the variable after call.
 *                 That is (elements - 1) if the complete chain is stored,
 *                 and (*waypoints_length - 1) if we only store some waypoints.
 *
 * @returns pointer to tail_element
 */
unsigned char *sha256_chain_with_waypoints(const unsigned char *seed,
                                           size_t seed_length,
                                           size_t elements,
                                           unsigned char *tail_element,
                                           sha256_chain_idx_elm_t *waypoints,
                                           size_t *waypoints_length);

/**
 * @brief function to verify if a given chain element is part of the chain.
 *
 * @param[in] element the chain element to be verified
 * @param[in] element_index the position in the chain
 * @param[in] tail_element the last element of the sha256-chain
 * @param[in] chain_length the number of elements in the chain
 *
 * @returns 0 if element is verified to be part of the chain at element_index
 *          1 if the element cannot be verified as part of the chain
 */
int sha256_chain_verify_element(unsigned char *element,
                                size_t element_index,
                                unsigned char *tail_element,
                                size_t chain_length);

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* _SHA256_H_ */
