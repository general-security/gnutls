typedef enum gnutls_privkey_pkcs8_flags {
	GNUTLS_PKCS8_PLAIN=1,  /* if set the private key will not
			        * be encrypted.
				*/
	GNUTLS_PKCS8_USE_PKCS12_3DES,
	GNUTLS_PKCS8_USE_PKCS12_ARCFOUR
} gnutls_privkey_pkcs8_flags;

int gnutls_x509_privkey_import(gnutls_x509_privkey key, const gnutls_datum * data,
	gnutls_x509_crt_fmt format);
ASN1_TYPE _gnutls_privkey_decode_pkcs1_rsa_key( const gnutls_datum *raw_key,
        gnutls_x509_privkey pkey);
int _gnutls_x509_hash_rsa_key( GNUTLS_MPI * params,
	unsigned char* output_data, int* output_data_size);
int _gnutls_x509_hash_dsa_key( GNUTLS_MPI * params,
	unsigned char* output_data, int* output_data_size);
