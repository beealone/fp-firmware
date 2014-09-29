#ifndef _JZ_GPIO_H_
#define _JZ_GPIO_H_

/*************************************************************************
 * Jz4730 GPIO register bits definition
 *************************************************************************/

#define GPIO_GPDR(n)	(jz_gpio_base + (0x00 + (n)*0x30))
#define GPIO_GPDIR(n)	(jz_gpio_base + (0x04 + (n)*0x30))
#define GPIO_GPODR(n)	(jz_gpio_base + (0x08 + (n)*0x30))
#define GPIO_GPPUR(n)	(jz_gpio_base + (0x0c + (n)*0x30))
#define GPIO_GPALR(n)	(jz_gpio_base + (0x10 + (n)*0x30))
#define GPIO_GPAUR(n)	(jz_gpio_base + (0x14 + (n)*0x30))
#define GPIO_GPIDLR(n)	(jz_gpio_base + (0x18 + (n)*0x30))
#define GPIO_GPIDUR(n)	(jz_gpio_base + (0x1c + (n)*0x30))
#define GPIO_GPIER(n)	(jz_gpio_base + (0x20 + (n)*0x30))
#define GPIO_GPIMR(n)	(jz_gpio_base + (0x24 + (n)*0x30))
#define GPIO_GPFR(n)	(jz_gpio_base + (0x28 + (n)*0x30))

#define REG_GPIO_GPDR(n)	VPint(GPIO_GPDR((n)))
#define REG_GPIO_GPDIR(n)	VPint(GPIO_GPDIR((n)))
#define REG_GPIO_GPODR(n)	VPint(GPIO_GPODR((n)))
#define REG_GPIO_GPPUR(n)	VPint(GPIO_GPPUR((n)))
#define REG_GPIO_GPALR(n)	VPint(GPIO_GPALR((n)))
#define REG_GPIO_GPAUR(n)	VPint(GPIO_GPAUR((n)))
#define REG_GPIO_GPIDLR(n)	VPint(GPIO_GPIDLR((n)))
#define REG_GPIO_GPIDUR(n)	VPint(GPIO_GPIDUR((n)))
#define REG_GPIO_GPIER(n)	VPint(GPIO_GPIER((n)))
#define REG_GPIO_GPIMR(n)	VPint(GPIO_GPIMR((n)))
#define REG_GPIO_GPFR(n)	VPint(GPIO_GPFR((n)))

#define GPIO_IRQ_LOLEVEL  0
#define GPIO_IRQ_HILEVEL  1
#define GPIO_IRQ_FALLEDG  2
#define GPIO_IRQ_RAISEDG  3

#define IRQ_GPIO_0	48
#define NUM_GPIO	128

#define GPIO_GPDR0      GPIO_GPDR(0)
#define GPIO_GPDR1      GPIO_GPDR(1)
#define GPIO_GPDR2      GPIO_GPDR(2)
#define GPIO_GPDR3      GPIO_GPDR(3)
#define GPIO_GPDIR0     GPIO_GPDIR(0)
#define GPIO_GPDIR1     GPIO_GPDIR(1)
#define GPIO_GPDIR2     GPIO_GPDIR(2)
#define GPIO_GPDIR3     GPIO_GPDIR(3)
#define GPIO_GPODR0     GPIO_GPODR(0)
#define GPIO_GPODR1     GPIO_GPODR(1)
#define GPIO_GPODR2     GPIO_GPODR(2)
#define GPIO_GPODR3     GPIO_GPODR(3)
#define GPIO_GPPUR0     GPIO_GPPUR(0)
#define GPIO_GPPUR1     GPIO_GPPUR(1)
#define GPIO_GPPUR2     GPIO_GPPUR(2)
#define GPIO_GPPUR3     GPIO_GPPUR(3)
#define GPIO_GPALR0     GPIO_GPALR(0)
#define GPIO_GPALR1     GPIO_GPALR(1)
#define GPIO_GPALR2     GPIO_GPALR(2)
#define GPIO_GPALR3     GPIO_GPALR(3)
#define GPIO_GPAUR0     GPIO_GPAUR(0)
#define GPIO_GPAUR1     GPIO_GPAUR(1)
#define GPIO_GPAUR2     GPIO_GPAUR(2)
#define GPIO_GPAUR3     GPIO_GPAUR(3)
#define GPIO_GPIDLR0    GPIO_GPIDLR(0)
#define GPIO_GPIDLR1    GPIO_GPIDLR(1)
#define GPIO_GPIDLR2    GPIO_GPIDLR(2)
#define GPIO_GPIDLR3    GPIO_GPIDLR(3)
#define GPIO_GPIDUR0    GPIO_GPIDUR(0)
#define GPIO_GPIDUR1    GPIO_GPIDUR(1)
#define GPIO_GPIDUR2    GPIO_GPIDUR(2)
#define GPIO_GPIDUR3    GPIO_GPIDUR(3)
#define GPIO_GPIER0     GPIO_GPIER(0)
#define GPIO_GPIER1     GPIO_GPIER(1)
#define GPIO_GPIER2     GPIO_GPIER(2)
#define GPIO_GPIER3     GPIO_GPIER(3)
#define GPIO_GPIMR0     GPIO_GPIMR(0)
#define GPIO_GPIMR1     GPIO_GPIMR(1)
#define GPIO_GPIMR2     GPIO_GPIMR(2)
#define GPIO_GPIMR3     GPIO_GPIMR(3)
#define GPIO_GPFR0      GPIO_GPFR(0)
#define GPIO_GPFR1      GPIO_GPFR(1)
#define GPIO_GPFR2      GPIO_GPFR(2)
#define GPIO_GPFR3      GPIO_GPFR(3)

/*************************************************************************
 * Jz4730 GPIO operations definition
 *************************************************************************/

/* p is the port number (0,1,2,3)
 * o is the pin offset (0-31) inside the port
 * n is the absolute number of a pin (0-124), regardless of the port
 * m is the interrupt manner (low/high/falling/rising)
 */

#define __gpio_port_data(p)	( REG_GPIO_GPDR(p) )

#define __gpio_port_as_output(p, o)		\
do {						\
    unsigned int tmp;				\
    REG_GPIO_GPIER(p) &= ~(1 << (o));		\
    REG_GPIO_GPDIR(p) |= (1 << (o));		\
    if (o < 16) {				\
	tmp = REG_GPIO_GPALR(p);		\
	tmp &= ~(3 << ((o) << 1));		\
	REG_GPIO_GPALR(p) = tmp;		\
    } else {					\
	tmp = REG_GPIO_GPAUR(p);		\
	tmp &= ~(3 << (((o) - 16)<< 1));	\
	REG_GPIO_GPAUR(p) = tmp;		\
    }						\
} while (0)

#define __gpio_port_as_input(p, o)		\
do {						\
    unsigned int tmp;				\
    REG_GPIO_GPIER(p) &= ~(1 << (o));		\
    REG_GPIO_GPDIR(p) &= ~(1 << (o));		\
    if (o < 16) {				\
	tmp = REG_GPIO_GPALR(p);		\
	tmp &= ~(3 << ((o) << 1));		\
	REG_GPIO_GPALR(p) = tmp;		\
    } else {					\
	tmp = REG_GPIO_GPAUR(p);		\
	tmp &= ~(3 << (((o) - 16)<< 1));	\
	REG_GPIO_GPAUR(p) = tmp;		\
    }						\
} while (0)

#define __gpio_as_output(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	__gpio_port_as_output(p, o);		\
} while (0)

#define __gpio_as_input(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	__gpio_port_as_input(p, o);		\
} while (0)

#define __gpio_set_pin(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	__gpio_port_data(p) |= (1 << o);	\
} while (0)

#define __gpio_clear_pin(n)			\
do {						\
	unsigned int p, o;			\
	p = (n) / 32;				\
	o = (n) % 32;				\
	__gpio_port_data(p) &= ~(1 << o);	\
} while (0)

static __inline__ unsigned int __gpio_get_pin(unsigned int n)
{
	unsigned int p, o;
	p = (n) / 32;
	o = (n) % 32;
	if (__gpio_port_data(p) & (1 << o))
		return 1;
	else
		return 0;
}

#endif /* _JZ_GPIO_H_ */

