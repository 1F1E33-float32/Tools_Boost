from typing import Optional

class cat_system_crypto:
    class MT:
        """Mersenne Twister random number generator"""
        def __init__(self, seed: int) -> None:
            """
            Initialize MT with a seed value.

            Args:
                seed: Initial seed value (uint32_t)
            """
            ...

        def genrand(self) -> int:
            """
            Generate a random number.

            Returns:
                Random uint32_t value
            """
            ...

    class Blowfish:
        """Blowfish cipher decryption"""
        def __init__(self, key: bytes) -> None:
            """
            Initialize Blowfish with a key.

            Args:
                key: Encryption key as bytes
            """
            ...

        def decrypt(self, ciphertext: bytes) -> bytes:
            """
            Decrypt ciphertext using Blowfish algorithm.

            Args:
                ciphertext: Encrypted data (must be multiple of 8 bytes)

            Returns:
                Decrypted plaintext as bytes

            Raises:
                RuntimeError: If ciphertext length is invalid
            """
            ...

class hca_decryptor:
    @staticmethod
    def decrypt(data: bytes, mainkey: int, subkey: Optional[int] = None) -> bytes:
        """
        Decrypt an HCA file to a new HCA with ciph=0, rebuilding header & per-frame CRCs.

        Args:
            data: Original .hca file content (bytes)
            mainkey: Base keycode (int)
            subkey: Optional subkey (int); combined as: key' = key * (((subkey<<16) | ((~subkey+2)&0xFFFF))) then low 56 bits

        Returns:
            bytes of the decrypted .hca file

        Raises:
            RuntimeError: If file is invalid or decryption fails
        """
        ...

__version__: str
__all__: list[str]
