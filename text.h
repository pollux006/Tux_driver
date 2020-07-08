/*
 * tab:4
 *
 * text.h - font data and text to mode X conversion utility header file
 *
 * "Copyright (c) 2004-2009 by Steven S. Lumetta."
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Steve Lumetta
 * Version:       2
 * Creation Date: Thu Sep  9 22:08:16 2004
 * Filename:      text.h
 * History:
 *    SL    1    Thu Sep  9 22:08:16 2004
 *        First written.
 *    SL    2    Sat Sep 12 13:40:11 2009
 *        Integrated original release back into main code base.
 */


#ifndef TEXT_H
#define TEXT_H

/* The default VGA text mode font is 8x16 pixels. */
#define FONT_WIDTH   8
#define FONT_HEIGHT 16

#define BAR_HEIGHT      18      // two pixel up and down
#define BAR_WIDTH       320        // same as the pixel of one row
#define BAR_WIDTH_MEM   BAR_WIDTH/4  //divide by four

#define BAR_DIM_SIZE        (BAR_WIDTH*BAR_HEIGHT)
#define BAR_BUF_SIZE        (BAR_WIDTH*FONT_HEIGHT)
#define BAR_BUF_ONE_PLANE   (BAR_DIM_SIZE/4)
#define MAX_NUM         (BAR_WIDTH/FONT_WIDTH)   // 40 is the max char we can put

#define CHAR_PER_BYTE   2           //
#define CAL_OFF(x)       (3 - (x & 3))      // plane offset
#define CAL_BIT_OFF(x)   (1-(x / 4))          // 4 bit is a byte   


#define TEXT_NUM        11 
/* Standard VGA text font. */
extern unsigned char font_data[256][16];


/**/
void generation_text(unsigned char* buf, char* str, unsigned char color1, unsigned char color2);
void generation_fruit_text(unsigned char* buf, char* str);

#endif /* TEXT_H */
