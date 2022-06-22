# enc28j60

If the ENS28J60 source files are not in the `components` directory of the ESP-IDF, the [CMakeLists.txt](../../CMakeLists.txt) will try to copy them. See `cmake .`

Otherwise copy the files manually:

```
enc28j60.h
esp_eth_enc28j60.h
esp_eth_mac_enc28j60.c
esp_eth_phy_enc28j60.c
```


## Support

For any issues related to wolfSSL or wolfSSH, please open an [issue](https://github.com/wolfssl/wolfssl/issues) on GitHub, 
visit the [wolfSSL support forum](https://www.wolfssl.com/forums/),
send an email to [support@wolfssl.com](mailto:support@wolfssl.com),   
or [contact us](https://www.wolfssl.com/contact/).