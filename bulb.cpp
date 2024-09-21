// license:GPLv3+
#include "stdafx.h"

#include <cmath>

// 2024.08.01 - Changes:
// * replace bulb characteristics by values based on real bulb resistance measures using the algorithm given at the end of this file
// * replace filament resistance model with a simpler one R = R0.(1 + 0.0045 (T-T0)) instead of R = R0 * (T/T0)^1.215 since it matches the model used to evaluate bulb characteristicvs and gives better results
// * add #906 bulb
//
// Testing guidelines:
// * Gun's Roses (Data East)'s bottom right white mars flasher should be pulsing (fading, not blinking) at the beginning of each ball
// * Twilight Zone's red flasher above slot machine should be blinking when a ball is ejected (not fading)
// * The Lord of The Ring should be fast blinking during attract, but slowly pulsing Sauron eye (fellowhip inserts as well when fellowship multiball ready)
// * Godzilla's helicopter inserts should be fading and blinking at the same time when the helicopter shot is made (side lane then opposite green target)

// Set to 1 to compute bulb characteristics (if enabled, this file can be copy/pasted and ran in any C interpreter)
#define ENABLE_COMPUTE_CHARACTERISTICS 0

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#if ENABLE_COMPUTE_CHARACTERISTICS
// Copied from bulb.h to allow running this as a single file in any C interpreter
#define BULB_44   0
#define BULB_47   1
#define BULB_86   2
#define BULB_89   3
#define BULB_906  4
#define BULB_MAX  5

#define BULB_T_MAX 3400

#else
#include "bulb.h"

#endif

/*-------------------
/  Bulb characteristics and precomputed LUTs
/-------------------*/
typedef struct {
   double rating_u; /* voltage rating for nominal operation */
   double rating_i; /* current rating for nominal operation */
   double rating_T; /* temperature rating for nominal operation */
   double surface;  /* filament surface in m², computed from previous ratings */
   double mass;     /* filament mass in kg, computed from previous ratings */
   double r0;       /* resistance at 293K, computed from previous ratings */
   double cool_factor[BULB_T_MAX + 1]; /* precomputed cool down factor = Energy / (Mass * Specific Heat) */
   double heat_factor[BULB_T_MAX + 1]; /* precomputed heat factor = 1.0 / (R * Mass * Specific Heat) */
} bulb_tLampCharacteristics;

// Impact of coil form factor approximated values based on "The Coiling Factor in the Tungsten Filament Lamps" by D. C. Agrawal
#define RESISTIVITY_COIL_FACTOR 0.97  // Impact on resistivity equation R = p.L/A
#define EMISSIVITY_COIL_FACTOR 0.6865 // Impact on emissivity (since part of the emitted energy hit back the filament)

// Impact of base and wire temperature convection based on "The Coiling Factor in the Tungsten Filament Lamps" by D. C. Agrawal
#define BASE_WIRE_LOSS 0.07

// Physic constants
#define STEPHAN_BOLTZMAN 5.670374419e-8  // Stephan Blotzman constant (W.m−2.K−4) used in Planck's law
#define RESISTIVITY_293K 5.65e-8         // Resistivity (Ohm.m) of tungsten at 293K
#define TUNGSTEN_DENSITY 19300.0         // tungsten density of 19300 g/m3

// Tungsten filament resistance depending on its base R0 and temperature
#define BULB_R(bulb, T) (bulbs[bulb].r0 * (1.0 + 0.0045 * (T - 293.0)))

/*-------------------
/  local variables
/-------------------*/
static struct {
  int initialized = FALSE;
  double                      t_to_p[BULB_T_MAX + 1 - 1500]; // Luminous flux in lumen as a function of the temperature
  double                      p_2700_to_t[512];
  double                      Tr; // Room temperature (defaults to T0 = 293K)
} locals;

static bulb_tLampCharacteristics bulbs[BULB_MAX] = {
   {  6.3, 0.250, 2710.0,  2.2191615654e-6, 0.3117866466e-6 }, //  #44 ( 54ms from 10% to 90%, 36ms cool down)
   {  6.3, 0.150, 2690.0,  1.3878130447e-6, 0.1402801155e-6 }, //  #47 ( 40ms from 10% to 90%, 26ms cool down)
   {  6.3, 0.200, 2550.0,  2.4286321482e-6, 0.3196915046e-6 }, //  #86 ( 64ms from 10% to 90%, 39ms cool down)
   { 13.0, 0.577, 2810.0,  8.8347591602e-6, 2.0675689639e-6 }, //  #89 ( 81ms from 10% to 90%, 55ms cool down)
   { 13.0, 0.690, 2755.0, 11.6609979870e-6, 3.1559940172e-6 }, // #906 (100ms from 10% to 90%, 67ms cool down)
};

// Linear RGB tint of a blackbody for temperatures ranging from 1500 to 3000K. The values are normalized for a relative luminance of 1.
// These values were evaluated using Blender's blackbody implementation, then normalized using the standard relative luminance formula (see https://en.wikipedia.org/wiki/Relative_luminance)
// We use a minimum channel value at 0.000001 to avoid divide by 0 in client application.
static constexpr float temperatureToTint[3 * 16] = {
   3.253114f, 0.431191f, 0.000001f,
   3.074210f, 0.484372f, 0.000001f,
   2.914679f, 0.531794f, 0.000001f,
   2.769808f, 0.574859f, 0.000001f,
   2.643605f, 0.612374f, 0.000001f,
   2.523686f, 0.645953f, 0.020487f,
   2.414433f, 0.676211f, 0.042456f,
   2.316033f, 0.703137f, 0.065485f,
   2.225598f, 0.727599f, 0.089456f,
   2.144543f, 0.749200f, 0.114156f,
   2.070389f, 0.768694f, 0.139412f,
   1.997974f, 0.787618f, 0.165180f,
   1.935725f, 0.803465f, 0.191508f,
   1.876871f, 0.818242f, 0.218429f,
   1.821461f, 0.832006f, 0.245241f,
   1.772554f, 0.843853f, 0.271900f 
};

/*-------------------------------
/  Initialize all pre-computed LUTs
/-------------------------------*/
void bulb_init()
{
   if (locals.initialized)
      return;
   locals.initialized = TRUE;

   #if !ENABLE_COMPUTE_CHARACTERISTICS
   locals.Tr = 293.0; // Defaults to a room temperature of 293K (20°C)
   #endif
   
   // Compute resistance at room temperature from other ratings U, I and T
   for (int bulb = 0; bulb < BULB_MAX; bulb++)
   {
     bulbs[bulb].r0 = 1.0;
     bulbs[bulb].r0 = (bulbs[bulb].rating_u / bulbs[bulb].rating_i) / BULB_R(bulb, bulbs[bulb].rating_T);
   }

   // Compute filament temperature to visible emission power (W.st-1.cm-2), according 
   // to the formula from "Luminous radiation from a black body and the mechanical equivalentt of light" by W.W.Coblentz and W.B.Emerson
   for (int i=0; i <= BULB_T_MAX - 1500; i++)
   {
      const double T = 1500.0 + i;
      locals.t_to_p[i] = 1.247/pow(1.0+129.05/T, 204.0) + 0.0678/pow(1.0+78.85/T, 404.0) + 0.0489/pow(1.0+23.52/T, 1004.0) + 0.0406/pow(1.0+13.67/T, 2004.0);
      locals.t_to_p[i] *= 68493.150685; // Convert from W.st-1.cm-2 to lumen.st-1.m-2
   }

   // Compute visible emission power to filament temperature LUT, normalized for a relative power of 255 for visible emission power at T=2700K
   // For the time being we simply search in the previously created LUT
   int t_pos = 0;
   double P2700 = locals.t_to_p[2700 - 1500];
   for (int i=0; i<512; i++)
   {
      const double p = i / 255.0;
      while (locals.t_to_p[t_pos] < p * P2700)
         t_pos++;
      locals.p_2700_to_t[i] = 1500 + t_pos;
   }

   // Precompute main parameters of the heating/cooldown model for each of the supported bulbs
   for (int i=0; i <= BULB_T_MAX; i++)
   {
      const double T = i;
      
      // Compute Tungsten specific heat in J.kg-1 (energy to temperature transfer, depending on temperature) according to formula from "Heating-times of tungsten filament incandescent lamps" by Dulli Chandra Agrawal
      //  Rg = 45.2268 J.kg-1.K-1 is gas constant for tungsten
      //  310 K is a constant called the Debye temperature for tungsten at room temperature
      const double specific_heat = 3.0 * 45.2268 * (1.0 - (310.0 * 310.0) / (20.0 * T*T)) + (2.0 * 4.5549e-3 * T) + (4.0 * 5.77874e-10 * T*T*T);
      
      // Compute cooldown and heat up factor for the predefined bulbs
      for (int bulb=0; bulb<BULB_MAX; bulb++)
      {
         // Radiated power in Watts, using Plank's law, corrected by the coil factor (to correct energy radiated that hit back the filament)
         const double emissivity = EMISSIVITY_COIL_FACTOR * 0.0000689 * pow(T, 1.0748); // tungsten emissivity (no dimension) over all wavelengths. Other paper gives (no significant impact): 0.0000664 * pow(T, 1.0796);
         bulbs[bulb].cool_factor[i] = - STEPHAN_BOLTZMAN * bulbs[bulb].surface * emissivity * (pow(T, 4.0) - pow(locals.Tr, 4.0));
         // Power lost by convection in the bulb base and wires, estimated as a percentage of the power at the bulb rating (note that this has minimal impact and could be neglected)
         bulbs[bulb].cool_factor[i] += - BASE_WIRE_LOSS * ((i - locals.Tr)/bulbs[bulb].rating_T) / BULB_R(bulb, T);
         // Electric power P=U²/R, but U² is not included in the precomputed LUT since it can be modulated
         bulbs[bulb].heat_factor[i] = 1.0 / BULB_R(bulb, T);
         // Pre-divide power by M.C
         bulbs[bulb].cool_factor[i] /= bulbs[bulb].mass * specific_heat;
         bulbs[bulb].heat_factor[i] /= bulbs[bulb].mass * specific_heat;
      }
   }
}

/*-------------------------------
/  Returns relative visible emission power for a given filament temperature. Result is relative to the emission power at the bulb rated stability temperature
/-------------------------------*/
float bulb_filament_temperature_to_emission(const int bulb, const float T)
{
   if (T < 1500.0f) return 0.f;
   if (T >= (float)BULB_T_MAX) return (float)locals.t_to_p[BULB_T_MAX - 1500];
   return (float)(locals.t_to_p[(int)T - 1500] / locals.t_to_p[(int)(bulbs[bulb].rating_T) - 1500]);
   // Linear interpolation is not worth its cost
   // int lower_T = (int) T, upper_T = lower_T+1;
   // float alpha = T - (float)lower_T;
   // return (1.0f - alpha) * (float)((locals.t_to_p[lower_T - 1500] + alpha * locals.t_to_p[upper_T - 1500])  / locals.t_to_p[(int)(bulbs[bulb].rating_T) - 1500]));
}

/*-------------------------------
// Returns linear RGB tint for a given filament temperature. The values are normalized for a perceived luminance of 1.
/-------------------------------*/
void bulb_filament_temperature_to_tint(const float T, float* linear_RGB)
{
   if (T < 1500.0f)
   {
      linear_RGB[0] = temperatureToTint[0];
      linear_RGB[1] = temperatureToTint[1];
      linear_RGB[2] = temperatureToTint[2];
   }
   else if (T >= 2999.0f)
   {
      linear_RGB[0] = temperatureToTint[15 * 3 + 0];
      linear_RGB[1] = temperatureToTint[15 * 3 + 1];
      linear_RGB[2] = temperatureToTint[15 * 3 + 2];
   }
   else
   {
      // Linear interpolation between the precomputed values
      float t_ref = (T - 1500.0f) * (float)(1./100.);
      int lower_T = (int)t_ref, upper_T = lower_T+1;
      float alpha = t_ref - (float)lower_T;
      linear_RGB[0] = (1.0f - alpha) * temperatureToTint[lower_T * 3 + 0] + alpha * temperatureToTint[upper_T * 3 + 0];
      linear_RGB[1] = (1.0f - alpha) * temperatureToTint[lower_T * 3 + 1] + alpha * temperatureToTint[upper_T * 3 + 1];
      linear_RGB[2] = (1.0f - alpha) * temperatureToTint[lower_T * 3 + 2] + alpha * temperatureToTint[upper_T * 3 + 2];
   }
}


/*-------------------------------
/  Returns filament temperature for a given visible emission power normalized for an emission power of 1.0 at 2700K
/-------------------------------*/
double bulb_emission_to_filament_temperature(const double p)
{
   const int v = (int)(p * 255.);
   return v >= 512 ? locals.p_2700_to_t[511] : locals.p_2700_to_t[v];
}

/*-------------------------------
/  Compute cool down factor of a filament
/-------------------------------*/
double bulb_cool_down_factor(const int bulb, const double T)
{
   return bulbs[bulb].cool_factor[(int) T];
}

/*-------------------------------
/  Compute cool down factor of a filament over a given period
/-------------------------------*/
double bulb_cool_down(const int bulb, double T, float duration)
{
   while (duration > 0.0f)
   {
      const float dt = duration > 0.001f ? 0.001f : duration;
      T += dt * bulbs[bulb].cool_factor[(int) T];
      #if !ENABLE_COMPUTE_CHARACTERISTICS
      if (T <= 294.0)
      {
         return 293.0;
      }
      #endif
      duration -= dt;
   }
   return T;
}

/*-------------------------------
/  Compute heat up factor of a filament under a given voltage (sum of heating and cooldown)
/-------------------------------*/
float bulb_heat_up_factor(const int bulb, const float T, const float U, const float serial_R)
{
   double U1 = U;
   if (serial_R != 0.f)
   {
      const double R = BULB_R(bulb, T);
      U1 *= R / (R + serial_R);
   }
   return (float)(U1 * U1 * bulbs[bulb].heat_factor[(int)T] + bulbs[bulb].cool_factor[(int)T]);
}

/*-------------------------------
/  Compute temperature of a filament under a given voltage over a given period (sum of heating and cooldown)
/-------------------------------*/
double bulb_heat_up(const int bulb, double T, float duration, const float U, const float serial_R)
{
   while (duration > 0.0f)
   {
      T = T < 293.0 ? 293.0 : T > BULB_T_MAX ? BULB_T_MAX : T; // Keeps T within the range of the LUT (between room temperature and melt down point)
      double U1 = U;
      if (serial_R != 0.f)
      {
         const double R = BULB_R(bulb, T);
         U1 *= R / (R + serial_R);
      }
      const double energy = U1 * U1 * bulbs[bulb].heat_factor[(int)T] + bulbs[bulb].cool_factor[(int)T];
      #if !ENABLE_COMPUTE_CHARACTERISTICS
      if (-10 < energy && energy < 10)
      {
         // Stable state reached since electric heat (roughly) equals radiation and base/wire losses cool down
         return T;
      }
      #endif
      float dt;
      if (energy > 1000e3)
      {
         // Initial current surge, 0.5ms integration period in order to account for the fast resistor rise that will quickly lower the current
         dt = duration > 0.0005f ? 0.0005f : duration;
      }
      else
      {
         // Ramping up, 1ms integration period
         dt = duration > 0.001f ? 0.001f : duration;
      }
      T += dt * energy;
      duration -= dt;
   }
   return T;
}

#if ENABLE_COMPUTE_CHARACTERISTICS

// The following code computes filament geometry parameters needed by the bulb 
// model from the bulb ratings U, I and T. The temperature T is usually not
// known, but it can be deduced by measuring the resistance of a cool unlit
// bulb at room temperature, R0. Then try admissible value for T and compare
// to the computed R0, then select the temperature for which computed and 
// measured resistances matches.
//
// The method used to evaluate the characteristics is the following:
//   . collect U and I ratings, compute R at stability temperature (T): R = U/I
//   . select a stability temperature (T), and compute corresponding R0 using the the 2 following equations:
//      R = p(T).L/A with p = resisitivity, L = filament length, A = wire section surface (Pi.r²)
//        we suppose L/A constant (limited dilatation), so R/R0 = p/p0 and therefore p = p0.R/R0.
//        knowing that p0 of tungsten is 5.65x10-8 Ohm.m at T0 = 293K
//      (p - p0) = a.p0.(T - T0) with a = temperature to resistivity of tungsten, approximated to 0.0045
//   . find (dichotomy search) a filament radius that leads to the chosen stability temperature (we compute the corresponding filament length since we know L/A = R0/p0 = L/(PI.r²) so L = PI.r².R0/p0)
//   . validate that the calculated values (corresponding to the selected stability temperature):
//      . (roughly) gives a R0 matching measures made wirh a multimeter on a set of real bulbs
//      . (roughly) matches the expected light intensity

#include <stdio.h>
#include <stdlib.h>

int fit_bulb(const int bulb, const int log)
{
   // search filament radius corresponding to bulb rating between 1 to 100 micrometer, using a simple dichotomy
   double rMin = 1.0e-6, rMax = 100.0e-6; 
   int n = 1;
   while (n < 50)
   {
      const double r = (rMin + rMax) * 0.5; // filament radius (dichotomy search)
      const double a = M_PI * r * r; // area of the filament section
      const double l = bulbs[bulb].r0 * RESISTIVITY_COIL_FACTOR * a / RESISTIVITY_293K; // length of filament, using R = p.L/A applied at T0 = 293K, applying a coil coefficient
      bulbs[bulb].surface = l * 2.0 * M_PI * r; // exterior surface of filament (radiating surface) computed as a linear cylinder (this could benefit from a coil factor)
      bulbs[bulb].mass = l * a * TUNGSTEN_DENSITY; // mass of filament

      // Force update of the precomputed LUTs
      initialized = 0;
      locals.Tr = 293.0;
      bulb_init();
      const double T = bulb_heat_up(bulb, 293.0, 100.0, bulbs[bulb].rating_u, 0.0);
      //printf("r(µm)=%f T=%f r0=%f l(mm)=%f s(mm²)=%f m(µg)=%f\n", r*1e6, T, bulbs[bulb].r0, l*1e3, bulbs[bulb].surface*1e6, bulbs[bulb].mass*1e6);

      if (abs(T - bulbs[bulb].rating_T) < 1e-4)
      {
         if (log)
         {
            printf("T  = %4.0f K (temperature of lit filament)\n", bulbs[bulb].rating_T);
            printf("R0 = %5.2f Ohms (resistance of unlit bulb at 293K)\n", bulbs[bulb].r0);
            printf("r  = %13.10f µm (radius of filament)\n", r*1e6);
            printf("l  = %13.10f mm (length of filament)\n", l*1e3);
            printf("s  = %13.10fe-6 m² (surface of filament)\n", bulbs[bulb].surface*1e6);
            printf("m  = %13.10fe-6 kg (mass of filament)\n", bulbs[bulb].mass*1e6);
            const double specific_heat = 3.0 * 45.2268 * (1.0 - (310.0 * 310.0) / (20.0 * T*T)) + (2.0 * 4.5549e-3 * T) + (4.0 * 5.77874e-10 * T*T*T);
            const double emitted_power = -bulb_cool_down_factor(bulb, T) * bulbs[bulb].mass * specific_heat;
            printf("Radiant  flux: %6.2f W (all wavelengths)\n", emitted_power);
            printf("Luminous flux: %6.2f lumen (perceived light)\n", emitted_power * locals.t_to_p[(int)T - 1500]);
         }
         return 1;
      }

      n++;
      if (T < bulbs[bulb].rating_T)
         rMax = r;
      else
         rMin = r;
   }
   return 0;
}

int main()
{
   locals.Tr = 293.0;
   bulb_init();

   for (int bulb = 0; bulb < BULB_MAX; bulb++)
   {
      const char* bulb_names[] = {"#44", "#47", "#86", "#89", "#906" };
      printf("Bulb: %s\n", bulb_names[bulb]);

      fit_bulb(bulb, 1);

      // Evaluate heating time to go from 10% to 90% of relative visible emission
      double T = 293.0, t = 0.0, t10 = 0.0, dt = 0.0005;
      int n = 1;
      while (n < 1000)
      {
         T = bulb_heat_up(bulb, T, dt, bulbs[bulb].rating_u, 0.0);
         t += dt;
         const float p = bulb_filament_temperature_to_emission(bulb, T);
         if (t10 == 0.0 && p >= 0.1)
            t10 = t;
         else if (t10 != 0.0 && p >= 0.9)
         {
            printf("Delay from 10%% to 90%%: %5.1fms\n", (t - t10)*1e3);
            break;
         }
      }

      // Evaluate cooling time to go from 90% to 10% of relative visible emission
      T = bulbs[bulb].rating_T;
      t = 0.0;
      t10 = 0.0;
      n = 1;
      while (n < 1000)
      {
         T = bulb_cool_down(bulb, T, dt);
         t += dt;
         const float p = bulb_filament_temperature_to_emission(bulb, T);
         if (t10 == 0.0 && p <= 0.9)
            t10 = t;
         else if (t10 != 0.0 && p <= 0.1)
         {
            printf("Delay from 90%% to 10%%: %5.1fms\n", (t - t10)*1e3);
            break;
         }
      }
       
      // Evaluate the impact of room temperature and multimeter current on the measured resistance (it applies of small voltage/current to measure which in turns very slightly heat and raise the resistance)
      // At their lowest resistance range, it looks like most multimetter inject around 1mA so we search for the corresponding resistor offset (Keysight U1232A, Fluke 87V, Brymen BM235, Extech EX330, Fluke 115, UNI-T UT61E)
      printf("Expected measure of unlit resistance (1mA multimeter):\n");
      const double prevT = bulbs[bulb].rating_T;
      for (int ratedT = 2200; ratedT <= 2800; ratedT += 50)
      {
         bulbs[bulb].rating_T = ratedT;
         fit_bulb(bulb, 0);

         printf(ratedT == prevT ? "*" : " ");
         printf("T=%dK", ratedT);
         for (int Tr = 293; Tr <= 303; Tr += 5)
         {
            initialized = 0;
            locals.Tr = Tr;
            bulb_init();
            double uMin = 0.001, uMax = 0.050; // Dichotomy between 1mV and 50mV
            n = 1;
            while (n < 50)
            {
               const double u = (uMin + uMax) * 0.5;
               T = bulb_heat_up(bulb, Tr, 100.0, u, 0.0);
               const double i = u / BULB_R(bulb, T);
               if (fabs(i - 0.001) < 1e-6)
               {
                  printf(" R=%4.2f (%d°C)", BULB_R(bulb, T), Tr - 273);
                  break;
               }
               n++;
               if (i > 0.001)
                  uMax = u;
               else
                  uMin = u;
            }
         }
         printf("\n");
      }
      bulbs[bulb].rating_T = prevT;
      printf("\n");
   }
    
   printf("Done.\n");
   return 0;
}

#endif