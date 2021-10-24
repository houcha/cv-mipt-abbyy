#pragma once

#include "bitmap_image.hpp"


void interpolate_green_for_red(rgb_t* p) {
  // R0   G1   R2   G3   R4
  // G5   B6   G7   B8   G9
  // R10  G11  R12  G13  R14
  // G15  B16  G17  B18  G19
  // R20  G21  R22  G23  R24

  uint32_t dN = abs(p[12].red - p[ 2].red)*2 + abs(p[17].green - p[ 7].green);
  uint32_t dE = abs(p[12].red - p[14].red)*2 + abs(p[11].green - p[13].green);
  uint32_t dW = abs(p[12].red - p[10].red)*2 + abs(p[13].green - p[11].green);
  uint32_t dS = abs(p[12].red - p[22].red)*2 + abs(p[ 7].green - p[17].green);

  uint32_t dMin = std::min({dN, dE, dW, dS});

  if (dN == dMin) {
    p[12].green = (p[ 7].green * 3 + p[17].green + p[12].red - p[ 2].red)/4;
  } else if (dE == dMin) {
    p[12].green = (p[13].green * 3 + p[11].green + p[12].red - p[14].red)/4;
  } else if (dW == dMin) {
    p[12].green = (p[11].green * 3 + p[13].green + p[12].red - p[10].red)/4;
  } else {
    p[12].green = (p[17].green * 3 + p[ 7].green + p[12].red - p[22].red)/4;
  }
}

void interpolate_green_for_blue(rgb_t* p) {
  // B0   G1   B2   G3   B4
  // G5   R6   G7   R8   G9
  // B10  G11  B12  G13  B14
  // G15  R16  G17  R18  G19
  // B20  G21  B22  G23  B24

  uint32_t dN = abs(p[12].blue - p[ 2].blue)*2 + abs(p[17].green - p[ 7].green);
  uint32_t dE = abs(p[12].blue - p[14].blue)*2 + abs(p[11].green - p[13].green);
  uint32_t dW = abs(p[12].blue - p[10].blue)*2 + abs(p[13].green - p[11].green);
  uint32_t dS = abs(p[12].blue - p[22].blue)*2 + abs(p[ 7].green - p[17].green);

  uint32_t dMin = std::min({dN, dE, dW, dS});

  if (dN == dMin) {
    p[12].green = (p[ 7].green * 3 + p[17].green + p[12].blue - p[ 2].blue)/4;
  } else if (dE == dMin) {
    p[12].green = (p[13].green * 3 + p[11].green + p[12].blue - p[14].blue)/4;
  } else if (dW == dMin) {
    p[12].green = (p[11].green * 3 + p[13].green + p[12].blue - p[10].blue)/4;
  } else {
    p[12].green = (p[17].green * 3 + p[ 7].green + p[12].blue - p[22].blue)/4;
  }
}

uint8_t hue_transit(uint8_t L1, uint8_t L2, uint8_t L3, uint8_t V1, uint8_t V3) {
  uint8_t V2 = 0;

  if (L1 < L2 && L2 < L3 || L1 > L2 && L2 > L3) {
    V2 = V1 + (V3 - V1) * (L2 - L1) / (L3 - L1);
  } else {
    V2 = (V1 + V3)/2 + (L2 - (L1 + L3)/2)/2;
  }

  return V2;
}

void interpolate_RB_for_green(rgb_t* p) {
  // G0 R1 G2
  // B3 G4 B5
  // G6 R7 G8

  p[4].blue = hue_transit(
      p[3].green, p[4].green, p[5].green, p[3].blue, p[5].blue);
  p[4].red = hue_transit(
      p[1].green, p[4].green, p[7].green, p[1].red, p[7].red);
}

void interpolate_blue_for_red(rgb_t* p) {
  // R0   G1   R2   G3   R4
  // G5   B6   G7   B8   G9
  // R10  G11  R12  G13  R14
  // G15  B16  G17  B18  G19
  // R20  G21  R22  G23  R24

  uint32_t dNE = abs(p[8].blue - p[16].blue)
      + abs(p[4].red - p[12].red) + abs(p[12].red - p[20].red)
      + abs(p[8].green - p[12].green) + abs(p[12].green - p[16].green);
  uint32_t dNW = abs(p[6].blue - p[18].blue)
      + abs(p[0].red - p[12].red) + abs(p[12].red - p[24].red)
      + abs(p[6].green - p[12].green) + abs(p[12].green - p[18].green);

  if (dNE < dNW) {
    p[12].blue = hue_transit(
        p[8].green, p[12].green, p[16].green, p[8].blue, p[16].blue);
  } else {
    p[12].blue = hue_transit(
        p[6].green, p[12].green, p[18].green, p[6].blue, p[18].blue);
  }
}

void interpolate_red_for_blue(rgb_t* p) {
  // B0   G1   B2   G3   B4
  // G5   R6   G7   R8   G9
  // B10  G11  B12  G13  B14
  // G15  R16  G17  R18  G19
  // B20  G21  B22  G23  B24

  uint32_t dNE = abs(p[8].red - p[16].red)
      + abs(p[4].blue - p[12].blue) + abs(p[12].blue - p[20].blue)
      + abs(p[8].green - p[12].green) + abs(p[12].green - p[16].green);
  uint32_t dNW = abs(p[6].red - p[18].red)
      + abs(p[0].blue - p[12].blue) + abs(p[12].blue - p[24].blue)
      + abs(p[6].green - p[12].green) + abs(p[12].green - p[18].green);

  if (dNE < dNW) {
    p[12].red = hue_transit(
        p[8].green, p[12].green, p[16].green, p[8].red, p[16].red);
  } else {
    p[12].red = hue_transit(
        p[6].green, p[12].green, p[18].green, p[6].red, p[18].red);
  }
}

