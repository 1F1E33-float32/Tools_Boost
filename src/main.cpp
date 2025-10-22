#include <nanobind/nanobind.h>

namespace nb = nanobind;

// Declare submodule initialization functions
void init_cat_system_crypto(nb::module_& m);
void init_hca_decryptor(nb::module_& m);

NB_MODULE(tools_boost, m)
{
    m.doc() = "Tools Boost";

    // Create submodules
    auto cat_crypto = m.def_submodule("cat_system_crypto", "CAT System crypto utilities");
    init_cat_system_crypto(cat_crypto);

    auto hca_dec = m.def_submodule("hca_decryptor", "HCA decryption utilities");
    init_hca_decryptor(hca_dec);
}
