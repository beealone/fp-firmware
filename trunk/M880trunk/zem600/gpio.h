#ifndef _JZ_GPIO_H_
#define _JZ_GPIO_H_

void __gpio_as_output(int n);
void __gpio_as_input(int n);
void __gpio_set_pin(int n);
void __gpio_clear_pin(int n);
int __gpio_get_pin(int n);
int gpio_open(void);

void Nand_read_onePage(U32 page, U8 *buf);
void Nand_write_onePage(U32 page, U8 *buf);
void Nand_read_oneBlock(U32 page, U8 *buf);
void Nand_write_oneBlock(U32 page, U8 *buf);
void Nand_erase_oneBlock(U32 block);
void Nand_save_MAC(U8 *buf);

/*dsl 2011.5.5*/
int Nand_Block_isBad(U32 block);

#endif /* _JZ_GPIO_H_ */
