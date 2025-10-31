#include <nanobind/nanobind.h>

// ============================================================================
// Module declarations
// ============================================================================

void init_catsystem2_decryptor(nanobind::module_ &m);
void init_hca_decryptor(nanobind::module_ &m);
void init_xpcm2pcm(nanobind::module_ &m);
void init_opusnx2opus(nanobind::module_ &m);

// ============================================================================
// Module initialization
// ============================================================================

NB_MODULE(tools_boost, m) {
    nanobind::module_ cat_crypto = m.def_submodule("catsystem2_decryptor", "CatSystem2 decryption utilities");
    init_catsystem2_decryptor(cat_crypto);

    nanobind::module_ hca_dec = m.def_submodule("hca_decryptor", "HCA decryption utilities");
    init_hca_decryptor(hca_dec);

    nanobind::module_ xpcm_ext = m.def_submodule("xpcm2pcm", "XPCM rebuild utilities");
    init_xpcm2pcm(xpcm_ext);

    nanobind::module_ opusnx = m.def_submodule("opusnx2opus", "Nintendo OPUS rebuild utilities");
    init_opusnx2opus(opusnx);
}
