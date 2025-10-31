from typing import Optional

class catsystem2_decryptor:
    class MT:
        def __init__(self, seed: int) -> None: ...
        def genrand(self) -> int: ...

    class Blowfish:
        def __init__(self, key: bytes) -> None: ...
        def decrypt(self, ciphertext: bytes) -> bytes: ...

class hca_decryptor:
    @staticmethod
    def hca_decryptor_main(data: bytes, mainkey: int, subkey: Optional[int] = None) -> bytes: ...

class xpcm2pcm:
    @staticmethod
    def xpcm2pcm_main(data: bytes) -> tuple[bytes, dict[str, int]]: ...

class opusnx2opus:
    @staticmethod
    def opusnx2opus_main(data: bytes) -> bytes: ...

__version__: str
__all__: list[str]
