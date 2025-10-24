from typing import Optional

class catsystem2_crypto:
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

class xpcm_extractor:
    @staticmethod
    def xpcm_to_pcm(xpcm_data: bytes) -> tuple[bytes, dict[str, int]]:
        """
        Decompress XPCM audio data to raw PCM format.

        Args:
            xpcm_data: XPCM compressed audio data (bytes)

        Returns:
            A tuple containing:
            - pcm_data (bytes): Raw PCM audio data
            - metadata (dict): Dictionary with audio metadata:
                * sample_rate (int): Sample rate in Hz
                * channels (int): Number of audio channels
                * bits_per_sample (int): Bit depth (usually 16)
                * num_samples (int): Total number of samples
                * pcm_size (int): Total size of PCM data in bytes
                * codec (int): XPCM codec identifier (0x00, 0x01, 0x02, 0x03, 0x05)
                * flags (int): XPCM flags

        Raises:
            RuntimeError: If XPCM data is invalid or decompression fails
        """
        ...

__version__: str
__all__: list[str]
