#ifndef _W25QXXCONFIG_H
#define _W25QXXCONFIG_H

/*
  Author:     Nima Askari
  WebSite:    http://www.github.com/NimaLTD
  Instagram:  http://instagram.com/github.NimaLTD
  Youtube:    https://www.youtube.com/channel/UCUhY7qY1klJm1d2kulr9ckw

  License:    GNU GPL v3, 29 June 2007		https://www.gnu.org/licenses/why-not-lgpl.html

  Version:    1.1.4


  Reversion History:

  (1.1.4)
  Fix W25qxx_IsEmptySector function.

  (1.1.3)
  Fix Erase and write sector in w25q256 and w25q512.

  (1.1.2)
  Fix read ID.

  (1.1.1)
  Fix some errors.

  (1.1.0)
  Fix some errors.

  (1.0.0)
  First release.
*/

#define _W25QXX_SPI                   hspi2
#define _W25QXX_CS_GPIO               LED_GPIO_Port
#define _W25QXX_CS_PIN                LED_Pin
#define _W25QXX_USE_FREERTOS          0
#define _W25QXX_DEBUG                 0

#endif
