/*
  Various LED Driver chips might need some specific code for initialisation/control logic

*/

#include <Arduino.h>
#include "ESP32-HUB75-MatrixPanel-DMA-leddrivers.h"
#include "ESP32-HUB75-MatrixPanel-DMA.h"

#define offset_prefix  dma_buff.all_row_data_cnt
#define offset_suffix  dma_buff.all_row_data_cnt + dma_buff.frame_prefix_cnt

enum{CLK_PULSE_DELAY = 1};
enum{CMD_DELAY = 10};
enum{LINE_DELAY = 50};

//array of commands for writing to registers
const uint8_t FM6363_REG_CMD[FM6363_REG_CNT] = {
    FM6363_WR_CFG1,
    FM6363_WR_CFG2,
    FM6363_WR_CFG3,
    FM6363_WR_CFG4,
    FM6363_WR_DBG,
};

//array of values ​​for commands to write to registers
const driver_rgb_t FM6363_REG_VALUE[FM6363_REG_CNT] = {
    {FM6363_CFG1_R,FM6363_CFG1_G,FM6363_CFG1_B},
    {FM6363_CFG2_R,FM6363_CFG2_G,FM6363_CFG2_B},
    {FM6363_CFG3_R,FM6363_CFG3_G,FM6363_CFG3_B},
    {FM6363_CFG4_R,FM6363_CFG4_G,FM6363_CFG4_B},
    {FM6363_DBG_x, FM6363_DBG_x, FM6363_DBG_x},
};


//set the sequence of bits in the buffer in the buffer
int data_set(ESP32_I2S_DMA_STORAGE_TYPE* buffer, int offset,  ESP32_I2S_DMA_STORAGE_TYPE data_mask, int len)
{
	while(len > 0)
	{
		buffer[offset^1] |= data_mask;
    offset++;
    len--;
	}
	return offset;
}

//reset the sequence of bits in the buffer in the buffer
int data_clr(ESP32_I2S_DMA_STORAGE_TYPE* buffer, int offset,  ESP32_I2S_DMA_STORAGE_TYPE data_mask, int len)
{
	while(len > 0)
	{
		buffer[offset^1] &= ~data_mask;
    offset++;
    len--;
	}
	return offset;
}

//setting the register\pixel value in the buffer
int setDataRegBuffer(ESP32_I2S_DMA_STORAGE_TYPE* buffer, int offset, const driver_rgb_t* regs_data)
{
  driver_reg_t r,g,b;
  ESP32_I2S_DMA_STORAGE_TYPE d;

	r = regs_data->r;
	g = regs_data->g;
	b = regs_data->b;

  for (int i = DRIVER_BITS; i > 0; i--)
	{
		d = 0;
		if (r & 0x8000) d |= BIT_R1|BIT_R2; r <<= 1;
		if (g & 0x8000) d |= BIT_G1|BIT_G2; g <<= 1;
		if (b & 0x8000) d |= BIT_B1|BIT_B2; b <<= 1;

    int j = offset^1;
		buffer[j] = (buffer[j] & ~BITMASK_RGB12) | d;
		offset++;
	}
  return offset;
}

//writing registers\filling a line of pixels in the buffer
int setDataRegBuffer_n(ESP32_I2S_DMA_STORAGE_TYPE* buffer, int offset, driver_rgb_t* regs_data, int regs_cnt)
{
	for(int i = regs_cnt-1; i >= 0; i--)
	{
    offset = setDataRegBuffer(buffer, offset, regs_data);
	}
  return offset;
}

//getting the mask of the string address value
ESP32_I2S_DMA_STORAGE_TYPE getAddrBits(uint8_t addr)
{
  ESP32_I2S_DMA_STORAGE_TYPE BIT_ADDR[ROW_ADDR_BITS] = {BIT_A, BIT_B, BIT_C, BIT_D, BIT_E};
  ESP32_I2S_DMA_STORAGE_TYPE res = 0;
  int i;
  for (i = 0; i < ROW_ADDR_BITS; i++)
  {
    if (addr & 1) res |= BIT_ADDR[i];
    addr >>= 1;
  }
  return res;
}

//setting LAT at the end of substrings in the buffer
int setLatRowBuffer(ESP32_I2S_DMA_STORAGE_TYPE* buffer, int offset, int subrow_cnt, int subrow_len)
{
  int i = offset + subrow_len-1;
  while(subrow_cnt > 0)
  {
    buffer[i^1] |= BIT_LAT;
    i += subrow_len + SUBROW_ADD_LEN;
    subrow_cnt--;
  }
  return offset;
}

//filling the buffer for regenerating lines in the buffer
//returns the frame start offsets (for joining buffers of different lengths to the suffix)
int FM6363setOEaddrBuffer(ESP32_I2S_DMA_STORAGE_TYPE* buffer, int offset, uint8_t row_cnt, size_t buffer_len, int frame_offset, bool decoder_INT595 = false)
{
  //return 0;
  enum {DELAY_INT595_START = 2};
  enum {DELAY_INT595_LEN = FM6363_ROW_OE_ADD_LEN/2-DELAY_INT595_START};
  enum {DELAY_INT595_CLK = FM6363_ROW_OE_ADD_LEN/4-DELAY_INT595_START};
  ESP32_I2S_DMA_STORAGE_TYPE data;
  ESP32_I2S_DMA_STORAGE_TYPE start_INT595 = 0;
  uint8_t addr;
  int row_offset;
  int oe_cnt;
  int clk_INT595_cnt = 255;  //clock duration in clock cycles
  //calculation of initial counter values
  if (frame_offset >= (FM6363_ROW_OE_LEN<<ROW_ADDR_BITS)) frame_offset = 0;
  addr = frame_offset/FM6363_ROW_OE_LEN;
  row_offset = frame_offset % FM6363_ROW_OE_LEN;
  if (row_offset > FM6363_ROW_OE_CNT*2) oe_cnt = 0;
  else oe_cnt = (FM6363_ROW_OE_CNT - (row_offset>>1));

  if(!decoder_INT595) data = getAddrBits(addr);
  else data = 0;

  while (offset < buffer_len)
  {
    if(row_offset == FM6363_ROW_OE_LEN)
    {
      addr++;
      if (addr >= row_cnt)
      {
        addr = 0;
        frame_offset = 0;
      }else if (addr == row_cnt - 1) //two channels: R1G1B1 and R2G2B2
      {
        start_INT595 = BIT_SDI;
      }
      if(!decoder_INT595) data = getAddrBits(addr);
      else data = 0;
      row_offset = 0;
      oe_cnt = FM6363_ROW_OE_CNT;
    }

    if(decoder_INT595)
    {
      if (oe_cnt > 0)
      {
        buffer[offset^1] = BIT_OE;
        oe_cnt--;
        if (oe_cnt == 0)
        {
          clk_INT595_cnt = -DELAY_INT595_START;
        }
      }else
      {
        if (clk_INT595_cnt <= DELAY_INT595_LEN)
        {
          clk_INT595_cnt++;
          if(clk_INT595_cnt >= 0)
          {
            if (clk_INT595_cnt == DELAY_INT595_LEN)
            {
              start_INT595 = 0;
              data = 0;
            }else
            {
              data = start_INT595 | BIT_RCK;
              if (clk_INT595_cnt >= DELAY_INT595_CLK) data |= BIT_DTK;
            }
          }
        }
        buffer[offset^1] = data;
      }
      row_offset++;
      frame_offset++;
      offset++;
      if (offset == buffer_len) break;
      buffer[offset^1] = data;
    }else
    {
      if (oe_cnt > 0)
      {
        buffer[offset^1] = data | BIT_OE;
        oe_cnt--;
      }else buffer[offset^1] = data;
      row_offset++;
      frame_offset++;
      offset++;
      if (offset == buffer_len) break;
      buffer[offset^1] = data;
    }

    row_offset++;
    frame_offset++;
    offset++;
  }
  return frame_offset++;
}

//set the frame header in the buffer
// This controls the LATCH line and sends:
// In case leds_enable = true:
//    PRE_ACT, EN_OP, V_SYNC, PRE_ACT
// In case leds_enable = false:
//    PRE_ACT, DIS_OP, PRE_ACT
int FM6363setVSyncBuffer(ESP32_I2S_DMA_STORAGE_TYPE* vsync_buffer, int offset, bool leds_enable, bool vsync)
{
	//if (vsync_buffer == NULL) return;
 	offset = data_clr(vsync_buffer, offset, BIT_LAT, FM6363_CMD_DELAY);
	offset = data_set(vsync_buffer, offset, BIT_LAT, FM6363_PRE_ACT);
	offset = data_clr(vsync_buffer, offset, BIT_LAT, FM6363_CMD_DELAY);
  int n = FM6363_CMD_DELAY;
  if (leds_enable)
  {
    offset = data_set(vsync_buffer, offset, BIT_LAT, FM6363_EN_OP);
    n += FM6363_DIS_OP - FM6363_EN_OP;
  }else
  {
    offset = data_set(vsync_buffer, offset, BIT_LAT, FM6363_DIS_OP);
  }
  offset = data_clr(vsync_buffer, offset, BIT_LAT, n);
  if (leds_enable)
	  offset = data_set(vsync_buffer, offset, BIT_LAT, FM6363_V_SYNC);
  else
    offset = data_clr(vsync_buffer, offset, BIT_LAT, FM6363_V_SYNC);
	offset = data_clr(vsync_buffer, offset, BIT_LAT, FM6363_CMD_DELAY);
  offset = data_set(vsync_buffer, offset, BIT_LAT, FM6363_PRE_ACT);
  offset = data_clr(vsync_buffer, offset, BIT_LAT, FM6363_CMD_DELAY);
  return offset;
}

//set the frame title
void MatrixPanel_DMA::FM6363setVSync(bool leds_enable, bool vsync)
{
  FM6363setVSyncBuffer(dma_buff.rowBits[offset_prefix], FRAME_ADD_LEN, leds_enable, vsync);
}

//setting the values ​​of configuration registers
void MatrixPanel_DMA::FM6363setReg(uint8_t reg_idx, driver_rgb_t* regs_data)
{
  // Sets the data for all registers?
  setDataRegBuffer_n(dma_buff.rowBits[offset_prefix], FRAME_ADD_LEN + FM6363_PREFIX_START_LEN, regs_data, driver_cnt);
  data_clr(dma_buff.rowBits[offset_prefix], dma_buff.frame_prefix_len - DRIVER_BITS, BIT_LAT, DRIVER_BITS);
  data_set(dma_buff.rowBits[offset_prefix], dma_buff.frame_prefix_len - FM6363_REG_CMD[reg_idx], BIT_LAT, FM6363_REG_CMD[reg_idx]);
}

//initialize buffers and descriptors
void MatrixPanel_DMA::FM6363initBuffers()
{

  //fill in the regeneration of rows in the data buffers
  #ifdef SERIAL_DEBUG
  Serial.print("DMA buffers init: rows  ");
  #endif
  int frame_offset_data = 0; //offset index relative to the beginning of the frame
  int row_offset = 0;
  for(int buf_id = m_cfg.double_dma_buff; buf_id >= 0; buf_id--)
  {
    frame_offset_data = 0;
    for(int row = 0; row < dma_buff.row_data_cnt; row++)
    {
      #ifdef SERIAL_DEBUG
      Serial.print(row); Serial.print(" ");
      #endif
      frame_offset_data = FM6363setOEaddrBuffer(dma_buff.rowBits[row_offset + row], 0, rows_per_frame, dma_buff.row_data_len, frame_offset_data, m_cfg.decoder_INT595);
      setLatRowBuffer(dma_buff.rowBits[row_offset + row],0,DRIVER_BITS,pixels_per_row);
    }
    dmadesc_data[buf_id][desc_data_cnt-1].eof = true;
    dmadesc_data[buf_id][desc_data_cnt-1].qe.stqe_next = &dmadesc_ext[FM6363_EXT_DATA];
    row_offset += dma_buff.row_data_cnt;
  }

  frame_offset_data %= dma_buff.frame_suffix_len;

  #ifdef SERIAL_DEBUG
  Serial.println("\r\nDMA buffers init: calculate ext descriptors");
  #endif
  frame_offset_data *= SIZE_DMA_TYPE;
  //find the descriptor where the current frame_offset is included in order to take buffer parameters from it
  int desk_idx_datain = frame_offset_data/DMA_MAX;
  //find the offset in the buffer to obtain the buffer address of the transition descriptor
  int offset_bufer_datain = frame_offset_data % DMA_MAX;

  //offset index relative to the beginning of the frame
  int frame_offset_prefix = FM6363setOEaddrBuffer(dma_buff.rowBits[offset_prefix], FRAME_ADD_LEN + FM6363_VSYNC_LEN,
                                                   rows_per_frame,dma_buff.frame_prefix_len,0, m_cfg.decoder_INT595);

  frame_offset_prefix %= dma_buff.frame_suffix_len;
  frame_offset_prefix *= SIZE_DMA_TYPE;
  //find the descriptor where the current frame_offset is included in order to take buffer parameters from it
  int desk_idx_prefixin = frame_offset_prefix/DMA_MAX;
  //find the offset in the buffer to obtain the buffer address of the transition descriptor
  int offset_bufer_prefixin = frame_offset_prefix % DMA_MAX;;
  //set the output of the prefix to the input of the suffix
  dmadesc_prefix[desc_prefix_cnt-1].eof= true;
  dmadesc_prefix[desc_prefix_cnt-1].qe.stqe_next = &dmadesc_ext[FM6363_EXT_PREFIX];

  #ifdef SERIAL_DEBUG
  Serial.println("DMA buffers init: oe_addr suffix buffers");
  #endif
  //fill the row regeneration buffer
  FM6363setOEaddrBuffer(dma_buff.rowBits[offset_suffix], 0, rows_per_frame, dma_buff.frame_suffix_len, 0, m_cfg.decoder_INT595);

  //int desk_idx_next;
  //fill the regeneration of lines in the suffix buffer
  for (int desc_suffix = FM6363_DSUFFIX_CNT-1; desc_suffix >= 0; desc_suffix--)
  {
    #ifdef SERIAL_DEBUG
    Serial.print("DMA descriptors init for suffix: "); Serial.println(desc_suffix);
    #endif
    //fill the row regeneration descriptors
    dmadesc_suffix[desc_suffix][desc_suffix_cnt-1].eof = true;
    dmadesc_suffix[desc_suffix][desc_suffix_cnt-1].qe.stqe_next = &dmadesc_suffix[desc_suffix][0];
  }

  #ifdef SERIAL_DEBUG
  Serial.println("DMA ext descriptor init for data");
  #endif
  //fill the input descriptor of the row data
  dmadesc_ext[FM6363_EXT_DATA].buf = &dmadesc_suffix[0][desk_idx_datain].buf[offset_bufer_datain];
  dmadesc_ext[FM6363_EXT_DATA].size = dmadesc_suffix[0][desk_idx_datain].size - offset_bufer_datain;
  dmadesc_ext[FM6363_EXT_DATA].length = dmadesc_ext[FM6363_EXT_DATA].size;
  //next descriptor after transition
  if (desk_idx_datain == (desc_suffix_cnt - 1))
  {
    //Serial.println("D");
    //dmadesc_ext[FM6363_EXT_DATA].eof = true;
    data_to_suffix = 0;
  }else data_to_suffix = desk_idx_datain + 1;
  dmadesc_ext[FM6363_EXT_DATA].qe.stqe_next = &dmadesc_suffix[0][data_to_suffix];

  #ifdef SERIAL_DEBUG
  Serial.println("DMA ext descriptor init for prefix: ");
  #endif
  //fill the prefix input descriptor
  dmadesc_ext[FM6363_EXT_PREFIX].buf = &dmadesc_suffix[0][desk_idx_prefixin].buf[offset_bufer_prefixin];
  dmadesc_ext[FM6363_EXT_PREFIX].size = dmadesc_suffix[0][desk_idx_prefixin].size - offset_bufer_prefixin;
  dmadesc_ext[FM6363_EXT_PREFIX].length = dmadesc_ext[FM6363_EXT_PREFIX].size;
  //next descriptor after transition
  if (desk_idx_prefixin == (desc_suffix_cnt - 1))
  {
    //Serial.println("P");
    //dmadesc_ext[FM6363_EXT_PREFIX].eof = true;
    prefix_to_suffix = 0;
  }else prefix_to_suffix = desk_idx_prefixin + 1;
  dmadesc_ext[FM6363_EXT_PREFIX].qe.stqe_next = &dmadesc_suffix[0][prefix_to_suffix];
}

void setDriverReg(driver_reg_t& driver_reg, driver_reg_t value, driver_reg_t mask, uint8_t offset)
{
  driver_reg = (driver_reg & (~mask))|((value << offset) & mask);
}

// This function overwrites the default register values, e.g. for setting linecount
void setDriverRegRGB(driver_rgb_t* driver_reg, driver_reg_t value, driver_reg_t mask, uint8_t offset)
{
  setDriverReg(driver_reg->r, value, mask, offset);
  setDriverReg(driver_reg->g, value, mask, offset);
  setDriverReg(driver_reg->b, value, mask, offset);
}

