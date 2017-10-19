
#include "asn/Condition.h"
#include "asn/Fulfillment.h"
#include "asn/Ed25519FingerprintContents.h"
#include "asn/OCTET_STRING.h"
#include "include/cJSON.h"
#include "cryptoconditions.h"


struct CCType cc_ed25519Type;


static char *ed25519Fingerprint(CC *cond) {
    Ed25519FingerprintContents_t fp;
    
    // Don't allocate memory for the buffer, the structure is on the stack so we'll borrow
    // a reference
    fp.publicKey.buf = cond->publicKey;
    fp.publicKey.size = 32;
    
    char out[BUF_SIZE];

    asn_enc_rval_t rc = der_encode_to_buffer(&asn_DEF_Ed25519FingerprintContents, &fp, out, BUF_SIZE);
    if (rc.encoded == -1) {
        return NULL; // TODO assert
    }
    char *hash = calloc(1, 32);
    crypto_hash_sha256(hash, out, rc.encoded);
    return hash;
}


static int ed25519VerifyMessage(CC *cond, char *msg, size_t length) {
    int rc = crypto_sign_verify_detached(cond->signature, msg, length, cond->publicKey);
    return rc == 0;
}


static unsigned long ed25519Cost(CC *cond) {
    return 131072;
}


static CC *ed25519FromJSON(cJSON *params, char *err) {
    cJSON *pk_item = cJSON_GetObjectItem(params, "publicKey");
    if (!cJSON_IsString(pk_item)) {
        strcpy(err, "publicKey must be a string");
        return NULL;
    }
    char *pk_b64 = pk_item->valuestring;
    size_t binsz;

    CC *cond = calloc(1, sizeof(CC));
    cond->type = &cc_ed25519Type;
    cond->publicKey = base64_decode(pk_b64, strlen(pk_b64), &binsz);
    cond->signature = NULL;
    return cond;
}


static void ed25519FfillToCC(Fulfillment_t *ffill, CC *cond) {
    cond->type = &cc_ed25519Type;
    cond->publicKey = calloc(1, 32);
    memcpy(cond->publicKey, ffill->choice.ed25519Sha256.publicKey.buf, 32);
    cond->signature = calloc(1, 64);
    memcpy(cond->signature, ffill->choice.ed25519Sha256.signature.buf, 64);
}


static void ed25519Free(CC *cond) {
    free(cond);
}


static uint32_t ed25519Subtypes(CC *cond) {
    return 0;
}


struct CCType cc_ed25519Type = { 4, "ed25519-sha-256", Condition_PR_ed25519Sha256, 0, &ed25519VerifyMessage, &ed25519Fingerprint, &ed25519Cost, &ed25519Subtypes, &ed25519FromJSON, &ed25519FfillToCC, &ed25519Free };
