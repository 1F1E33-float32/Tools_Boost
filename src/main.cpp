#include <nanobind/nanobind.h>

// ============================================================================
// Module declarations
// ============================================================================

void init_catsystem2_crypto(nanobind::module_& m);
void init_hca_decryptor(nanobind::module_& m);
void init_xpcm_extractor(nanobind::module_& m);

// ============================================================================
// Module initialization
// ============================================================================

NB_MODULE(tools_boost, m)
{
    m.doc() = "Tools Boost";

    nanobind::module_ cat_crypto = m.def_submodule("catsystem2_crypto", "CatSystem2 crypto utilities");
    init_catsystem2_crypto(cat_crypto);

    nanobind::module_ hca_dec = m.def_submodule("hca_decryptor", "HCA decryption utilities");
    init_hca_decryptor(hca_dec);

    nanobind::module_ xpcm_ext = m.def_submodule("xpcm_extractor", "XPCM extraction utilities");
    init_xpcm_extractor(xpcm_ext);
}
