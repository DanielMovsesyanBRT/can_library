/**
 *
 * Author : Author Daniel Movsesyan
 * Created On : 11/16/2020 
 * File : can_constants.hpp
 *
 */

#pragma once

namespace brt {
namespace can {

#define MAX_CAN_PACKET_SIZE                 (8)
#define DEFAULT_CAN_PRIORITY                (6)
#define BROADCATS_CAN_ADDRESS               (255)
#define NULL_CAN_ADDRESS                    (254)

/**
 * \enum PGNs
 *
 */
enum PGNs
{
  PGN_AckNack             = 0xE800, // 59392
  PGN_Request             = 0xEA00, // 59904

  PGN_TP_DT               = 0xEB00, // 60160  - Transport Protocol Data Transfer
  PGN_TP_CM               = 0xEC00, // 60416  - Transport Protocol Connection Management

  PGN_AddressClaimed      = 0xEE00, // 60928
  PGN_ProprietaryA        = 0xEF00, // 61184

  PGN_DiagnosticProtocol  = 0xFD32, // 64818
  PGN_ECUID               = 0xFDC5, // 64965
  PGN_SoftwareID          = 0xFEDA, // 65242
  PGN_Timedate            = 0xFEE6, // 65254

  PGN_ProprietaryB_start  = 0xFF00,
  PGN_ProprietaryB_end    = 0xFFFF,

  PGN_ProprietaryA2       = 0x1EF00, // 126720
};

/**
 * \enum IndustryGroup
 *
 */
enum IndustryGroup
{
  ig_Global = 0,
  ig_OnHighwayEquipment = 1,
  ig_AgriculturalAndForestry = 2,
  if_Construction = 3,
  if_Marine = 4,
  if_IndustrialProcessControlStationary = 5
};

/**
 * \enum DeviceClasses
 *
 */
enum DeviceClasses
{
  dc_NonSpecificSystem = 0,
  dc_Tractor = 1,
  dc_Tillage = 2,
  dc_SecondaryTillage = 3,
  dc_PlanterSeeder = 4,
  dc_Fertilizers = 5,
  dc_Sprayers = 6,
  dc_Harvesters = 7,
  dc_RootHarvester = 8,
  dc_Forage = 9,
  dc_Irrigation = 10,
  dc_TransportTrailer = 11,
  dc_FarmYardOperation = 12,
  dc_PoweredAUXDevice = 13,
  dc_SpecialCrops = 14,
  dc_EarthWork = 15,
  dc_Skidder = 16,
  dc_SensorSystem = 17,
  dc_TimberHarvester = 19,
  dc_Forwarder = 20,
  dc_TimberLoader = 21,
  dc_TimberProcessingMachine = 22,
  dc_Mulcher = 23,
  dc_UtilityVehicle = 24,
  dc_SlurryManuteApplicator = 25,
  dc_FeederMixer = 26,
  dc_Weeder = 27
};

/**
 * \enum MFCodes
 *
 */
enum MFCodes
{
  mf_Reserved = 0,
  mf_Bendix_Commercial_Vehicle_Systems_LLC = 1,
  mf_Allison_Transmission_Inc = 2,
  mf_Ametek_US_Gauge_Division = 3,
  mf_Ametek_Dixson = 4,
  mf_AMP_Inc = 5,
  mf_Berifors_Electronics_AB = 6,
  mf_Case_Corp = 7,
  mf_Caterpillar_Inc = 8,
  mf_Chrysler_Corp = 9,
  mf_Cummins_Inc = 10,
  mf_Dearborn_Group_Inc = 11,
  mf_Deere_and_Company_Precision_Farming = 12,
  mf_Delco_Electronics = 13,
  mf_Detroit_Diesel_Corporation = 14,
  mf_DICKEY_john_Corporation = 15,
  mf_Eaton_Corp = 16,
  mf_Eaton_Corp_Corp_Res_and_Dev = 17,
  mf_Eaton_Corp_Transmission_Div = 18,
  mf_Eaton_Corp_Trucking_Info_Services = 19,
  mf_Eaton_Ltd = 20,
  mf_Echlin_Inc_Midland_Brake_Inc = 21,
  mf_Ford_Motor_Co_Electronic_Concepts_and_Systems = 22,
  mf_Ford_Motor_Co_Heavy_Truck = 23,
  mf_Ford_Motor_Co_Vehicle_Controls = 24,
  mf_Daimler_Trucks_North_America_LLC = 25,
  mf_General_Motors_Corp_Service_Technology_Grp = 26,
  mf_GMC = 27,
  mf_Grote_Ind_Inc = 28,
  mf_Hino_Motors_Ltd = 29,
  mf_Isuzu_Motors_Ltd = 30,
  mf_J_Pollak_Corp = 31,
  mf_Jacobs_Vehicle_Systems = 32,
  mf_John_Deere = 33,
  mf_Kelsey_Hayes_Co = 34,
  mf_Kenworth_Truck_Co = 35,
  mf_Lucas_Ind = 36,
  mf_Mack_Trucks_Inc = 37,
  mf_Micro_Processor_Systems_Inc = 38,
  mf_Microfirm_Inc = 39,
  mf_Motorola_AIEG_Inc = 40,
  mf_Motorola_Inc = 41,
  mf_International_Truck_and_Engine_Corporation_Engine_Electronics_ = 42,
  mf_International_Truck_and_Engine_Corporation_Vehicle_Electronics = 43,
  mf_Nippondenso_Co_Ltd = 44,
  mf_PACCAR = 45,
  mf_Noregon_Systems_Inc_ = 46,
  mf_Phillips_Semiconductor = 47,
  mf_Pollak_Alphabet = 48,
  mf_RE_America_Inc = 49,
  mf_Robert_Bosch_Corp = 50,
  mf_Robert_Bosch_GmbH = 51,
  mf_Meritor_Automotive_Inc = 52,
  mf_Continental_Automotive_Systems_US_Inc = 53,
  mf_Meritor_Wabco = 54,
  mf_Ryder_System_Inc = 55,
  mf_SAIC = 56,
  mf_Danfoss_ = 57,
  mf_SPX_Corporation_OTC_Div = 58,
  mf_VES_Inc = 59,
  mf_Volvo_Trucks_North_America_Inc = 60,
  mf_Volvo_Truck_Corp = 61,
  mf_Wabco = 62,
  mf_ZF_Industries_Inc = 63,
  mf_MAN_Nutzfahrzeuge_AG = 65,
  mf_John_Deere_Construction_Equipment_Division = 66,
  mf_John_Deere_Coffeyville_Works = 67,
  mf_Scania = 68,
  mf_Trimble_Navigation = 69,
  mf_Flex_coil_Limited = 70,
  mf_Vansco_Electronics_Ltd = 71,
  mf_Sisu_Corporation = 72,
  mf_LeTourneau_Inc = 73,
  mf_Eaton_Axle_Brake_Division = 74,
  mf_Deere_and_Co_Agricultural_Division = 75,
  mf_Deere_Power_Systems_Group = 77,
  mf_Frank_W_Murphy_Manufacturing_Inc = 78,
  mf_Daimler_Benz_AG_Engine_Division = 79,
  mf_Twin_Disc_Inc = 80,
  mf_Fire_Research_Corp = 81,
  mf_Bobcat_Ingersoll_Rand = 82,
  mf_Bendix_VORAD_Technologies = 83,
  mf_New_Holland_UK_Limited = 84,
  mf_Kohler_Co = 85,
  mf_C_E_Niehoff_and_Company = 86,
  mf_JC_Bamford_Excavators_Ltd = 87,
  mf_Hemisphere_GPS_Inc = 88,
  mf_Kverneland_Group_Electronics_Division = 89,
  mf_Knorr_Bremse_SfN_GmbH = 90,
  mf_BSG_Bodensee_Steuergeraete_GmbH = 91,
  mf_Ag_Chem_Equipment_Co_Inc = 92,
  mf_Perkins_Engines_Company_Ltd = 93,
  mf_CNH_Industrial_NV = 94,
  mf_Pacific_Insight_Electronics_Corp = 95,
  mf_Mech_tronic_IT_GmbH = 96,
  mf_Ag_Leader_Technology_Inc = 97,
  mf_Mueller_Elektronik_GmbH_and_Co = 98,
  mf_International_Transmissions_Ltd = 99,
  mf_VDO_Technik_AG = 100,
  mf_Sensoria = 101,
  mf_AGCO = 102,
  mf_CLAAS_E_Systems_GmbH = 103,
  mf_CLAAS_KGaA_mbH = 104,
  mf_Kiepe_Elektrik_GmbH_and_Co_KG = 105,
  mf_BAE_Systems_Controls_Inc = 106,
  mf_Grimme_Landmaschinen_GmbH_and_Co_KG = 107,
  mf_WTK_Elecktronik_GmbH = 108,
  mf_TeeJet_Technologies_Denmark = 109,
  mf_EPIQ_Sensor_Nite = 110,
  mf_Bernard_Krone_Holding_SE_and_Co_KG = 111,
  mf_MECALAC = 112,
  mf_Stress_Tek_Inc = 113,
  mf_EControls_Inc = 114,
  mf_NACCO_Materials_Handling_Group_Inc = 115,
  mf_BEELINE_Technologies = 116,
  mf_HUSCO_International = 117,
  mf_Intron_GmbH = 118,
  mf_IntegriNautics_ = 119,
  mf_RDS_Technology_Ltd = 120,
  mf_HED = 121,
  mf_FG_Wilson_Limited = 122,
  mf_Basler_Electric = 123,
  mf_Hydac_Electronic = 124,
  mf_Nevada_Automotive_Test_Center = 125,
  mf_Driver_Tech = 126,
  mf_Holland_USA = 127,
  mf_Gerhard_Duecker_GmbH_and_Co_KG = 128,
  mf_OMNEX_Control_Systems_Inc = 129,
  mf_Nido_Universal_Machines_BV = 130,
  mf_ITT_Industries = 131,
  mf_Mulag_Fahrzeugwerk = 132,
  mf_Bucher_Schoerling_GmbH = 133,
  mf_Iris_Technology_Ltd = 134,
  mf_Airmar_Technology_Corporation = 135,
  mf_Komatsu_Ltd = 136,
  mf_Maretron = 137,
  mf_Georg_Fritzmeier_GmbH_and_Co_KG = 138,
  mf_Caterpillar_Trimble_Control_Technologies_LLC = 139,
  mf_Lowrance_Electronics_Inc = 140,
  mf_Thales_Navigation_Ltd = 141,
  mf_TRW_Automotive = 142,
  mf_W_Gmeiner_GmbH_and_Co = 143,
  mf_Mercury_Marine = 144,
  mf_MurCal_Controls = 145,
  mf_Maxima_Technologies = 146,
  mf_Nautibus_electronic_GmbH = 147,
  mf_Blue_Water_Data_Inc = 148,
  mf_Holset = 149,
  mf_Fleetguard = 150,
  mf_Raven_Industries_Inc = 151,
  mf_elobau_GmbH_and_Co_KG = 152,
  mf_Woodward_Industrial_Controls_Division = 153,
  mf_Westerbeke_Corporation = 154,
  mf_Vetronix_Corporation = 155,
  mf_ITT_Industries_Cannon = 156,
  mf_ISSPRO_Inc = 157,
  mf_Firestone_Industrial_Products_Company = 158,
  mf_NTech_Industries_Inc = 159,
  mf_Nido = 160,
  mf_Offshore_Systems_Ltd = 161,
  mf_Axiomatic_Technologies = 162,
  mf_BRP_Inc = 163,
  mf_MTU_Friedrichshafen_GmbH = 164,
  mf_CPAC_Systems_AB = 165,
  mf_John_Deere_Electronic_Solutions = 166,
  mf_JLG_Industries_Inc = 167,
  mf_Xantrex = 168,
  mf_Marlin_Technologies_Inc = 169,
  mf_Computronics_Corporation_Ltd = 170,
  mf_Topcon_Electronics_GmbH_and_Co_KG = 171,
  mf_Yanmar_Co_Ltd = 172,
  mf_Ryeso_Inc = 173,
  mf_AB_Volvo_Penta = 174,
  mf_Veris_Technologies_Inc = 175,
  mf_Moritz_Aerospace = 176,
  mf_Diagnostic_Systems_Associates = 177,
  mf_Continental_Automotive_GmbH = 178,
  mf_TeeJet_Technologies_Springfield = 179,
  mf_Smart_Power_Systems = 180,
  mf_Coretronics_Inc = 181,
  mf_Vehicle_Systems_Engineering_BV = 182,
  mf_KDS_Controls_Inc = 183,
  mf_EIA_Electronics = 184,
  mf_Beede_Electrical_Instrument_Company = 185,
  mf_Altronic_Inc = 186,
  mf_Air_Weigh = 187,
  mf_EMP_Corp = 188,
  mf_QUALCOMM = 189,
  mf_Hella_KGaA_Hueck_and_Co = 190,
  mf_XRS_Corporation = 191,
  mf_Floscan = 192,
  mf_Jeppesen_Marine = 193,
  mf_TriMark_Corporation = 194,
  mf_General_Engine_Products = 195,
  mf_LEMKEN_GmbH_and_Co_KG = 196,
  mf_Mechron_Power_Systems = 197,
  mf_Mystic_Valley_Communications = 198,
  mf_ACTIA_Group = 199,
  mf_MGM_Brakes = 200,
  mf_Disenos_y_Tecnologia_SA = 201,
  mf_Curtis_Instruments_Inc = 202,
  mf_MILtronik_GmbH = 203,
  mf_The_Morey_Corporation = 204,
  mf_SmarTire_Systems_Inc = 205,
  mf_port_GmbH = 206,
  mf_Otto_Engineering = 207,
  mf_Drew_Technologies_Inc = 208,
  mf_Bell_Equip_Co_SA_LTD = 209,
  mf_Iteris_Inc = 210,
  mf_DNA_Group = 211,
  mf_Sure_Power_Industries_Inc = 212,
  mf_CNH_Belgium_NV = 213,
  mf_MC_elettronica_Srl = 214,
  mf_Aetna_Engineering_Fireboy_Xintex = 215,
  mf_Paneltronics_Inc = 216,
  mf_RM_Michaelides_Software_and_Elektronik_GmbH = 217,
  mf_Gits_Manufacturing_Company = 218,
  mf_Cat_OEM_Solutions = 219,
  mf_Beede_Electrical_Instrument_Company_Inc = 220,
  mf_SiE = 221,
  mf_Generac_Power_Systems_Inc = 222,
  mf_Vaueo_Retarder_Co_Ltd = 223,
  mf_EMMI_Network_SL = 224,
  mf_SKF = 225,
  mf_Monaco_Coach_Corporation = 226,
  mf_Lykketronic_A_S = 227,
  mf_Garmin_International_Inc = 229,
  mf_Saucon_Technologies = 230,
  mf_Topcon_Positioning_Systems_Inc = 231,
  mf_TSD_Integrated_Controls = 232,
  mf_Yacht_Monitoring_Solutions_Inc = 233,
  mf_Mondial_electronic_GmbH = 234,
  mf_SailorMade_Marine_Telemetry_Tetra_Technology_Ltd = 235,
  mf_NORAC_Systems_International_Inc = 236,
  mf_Agtron_Enterprises_Inc = 237,
  mf_ZF_Friedrichshafen_AG = 238,
  mf_May_and_Scofield_Ltd = 239,
  mf_Vanair_Mfg = 240,
  mf_Schneider_Automation_SAS = 241,
  mf_Kokusandenki_Co_Ltd = 242,
  mf_eRide_Inc = 243,
  mf_Techno_Matic = 244,
  mf_Capstan_Ag_Systems_Inc = 245,
  mf_Class_1_Inc = 246,
  mf_ePULSE = 247,
  mf_Cooper_Standard_Automotive_Active_Systems_Group = 248,
  mf_Schaltbau_GmbH = 249,
  mf_Kuhn_Group = 250,
  mf_German_Agricultural_Society_Test_Center = 251,
  mf_Sensor_Technik_Wiedemann_GmbH = 252,
  mf_Mobile_Control_Systems = 253,
  mf_GE_Sensing = 254,
  mf_MEAS_France = 255,
  mf_Tyco_Electronics_AMP = 256,
  mf_Honda_Motor_Co_Ltd = 257,
  mf_ARAG = 258,
  mf_Jetter_AG = 259,
  mf_Reichhardt_GmbH_Steuerungstechnik = 260,
  mf_Red_Dot_Corporation = 261,
  mf_HydraForce_Inc = 262,
  mf_IMMI = 263,
  mf_Autolync = 264,
  mf_MTS_Sensor_Technologie_GmbH = 265,
  mf_International_Thermal_Research_Ltd = 266,
  mf_Red_Lion_Controls_Inc = 267,
  mf_Accurate_Technologies = 268,
  mf_Saft_America_Inc_Space_and_Defense_Division = 269,
  mf_Tennant = 270,
  mf_Cole_Hersee = 271,
  mf_Gross_Mechanical_Laboratories_Inc = 272,
  mf_Active_Research_Limited = 273,
  mf_LTW_Technology_Co_LTD = 274,
  mf_Navico_Egersund_AS = 275,
  mf_Aqua_Hot_Heating_Systems = 276,
  mf_LHP_Telematics = 277,
  mf_Takata_Electronics = 278,
  mf_Geometris_LP = 279,
  mf_Leica_Geosystems_Pty_Ltd = 280,
  mf_Precision_Governors_LLC = 281,
  mf_Medallion_Instrumentation_Systems = 282,
  mf_CWF_Hamilton_and_Co_Ltd = 283,
  mf_Mobile_Tech_Chile = 284,
  mf_Sea_Recovery_Corp = 285,
  mf_Coelmo_srl = 286,
  mf_NTech_Industries_Inc_2 = 287,
  mf_Mitsubishi_FUSO_Truck_and_Bus_Corp = 288,
  mf_Watlow = 289,
  mf_Kuebler_GmbH = 290,
  mf_Groeneveld_Transport_Efficiency_BV = 291,
  mf_IKUSI_Angel_Iglesias_SA = 292,
  mf_Spyder_Controls_Corp = 293,
  mf_Grayhill_Inc = 294,
  mf_BEP_Marine = 295,
  mf_micro_dynamics_GmbH = 296,
  mf_Zonar_Systems_Inc = 297,
  mf_Holley_Performance = 298,
  mf_Rauch = 299,
  mf_Systron_Donner_Automotive = 300,
  mf_Parker_Hannifin_Ltd_FDE_group = 301,
  mf_Nissin_Kogyo_Co_LTD = 302,
  mf_CTS_Corporation = 303,
  mf_EmpirBus_AB = 304,
  mf_NovAtel_Inc = 305,
  mf_Sleipner_Motor_AB = 306,
  mf_MAS_Technologies = 307,
  mf_Cyntrx = 308,
  mf_Krauss_Maffei_Wegmann_GmbH_and_Co_KG = 309,
  mf_TECNORD_srl = 310,
  mf_Patrick_Power_Products = 311,
  mf_Lectronix_Inc = 312,
  mf_Ilmor_Engineering_Inc = 313,
  mf_CSM_GmbH = 314,
  mf_Icom_Incorporated = 315,
  mf_ITT_Flow_Control = 316,
  mf_Navtronics_Bvba = 317,
  mf_SAT_Plan = 318,
  mf_Cadec_Global = 319,
  mf_Miedema_Landbouwwerktuigenfabriek_BV = 320,
  mf_Ultra_Electronics_Electrics = 321,
  mf_MICHENKA_sro = 322,
  mf_Mobileye_Vision_Technologies_Ltd = 323,
  mf_Snap_on_Diagnostics = 324,
  mf_ASM_Automation_Sensorik_Messtechnik_GmbH = 325,
  mf_Akron_Brass_Company = 326,
  mf_Sonceboz_SA = 327,
  mf_Qwerty_Electronik_AB = 328,
  mf_Deif_A_S = 329,
  mf_Kidde_Aerospace_and_Defense = 330,
  mf_Horton_Inc = 331,
  mf_HWH_Corporation = 332,
  mf_Hadley_Products_Corporation = 333,
  mf_Takata_Petri_AG = 334,
  mf_Evo_Electric_Ltd = 335,
  mf_APE_sro = 336,
  mf_Carraro_SpA = 337,
  mf_GRAF_SYTECO = 338,
  mf_Competence_Center_ISOBUS_eV = 339,
  mf_Continental_AG = 340,
  mf_Boning_GmbH_and_Co = 341,
  mf_THOMAS_MAGNETE_GmbH = 342,
  mf_Baumer_Group = 343,
  mf_Parvus_Corporation = 344,
  mf_Korean_Maritime_University = 345,
  mf_Control_Solutions = 346,
  mf_Honeywell = 347,
  mf_Amazonen_Werke_H_Dreyer = 348,
  mf_Suonentieto = 349,
  mf_Noris_Marine_Systems_GmbH_and_Co_KG = 350,
  mf_Thrane_and_Thrane = 351,
  mf_SAME_DEUTZ_FAHR_GROUP_SpA = 352,
  mf_Hegemon_Electronics_Inc = 353,
  mf_Junkkari_OY = 354,
  mf_Mastervolt_International_BV = 355,
  mf_Fischer_Panda_Generators_Inc = 356,
  mf_Hardi_International_AS = 357,
  mf_Victron_Energy_BV = 358,
  mf_Ludwig_Bergmann_GmbH = 359,
  mf_HJS_Emission_Technology_GmbH_and_Co_KG_ = 360,
  mf_InMach = 361,
  mf_Poettinger_Landtechnik_GmbH = 362,
  mf_BEI_Duncan = 363,
  mf_OEM_Controls_Inc = 364,
  mf_Digi_Star_LLC = 365,
  mf_Viewnyx_Corp = 366,
  mf_Fliegl_Agrartechnik = 367,
  mf_HANSENHOF_electronic = 368,
  mf_Power_Torque_Engineering_Ltd = 369,
  mf_Rolls_Royce_Marine_AS = 370,
  mf_Heinzmann_GmbH_and_Co_KG = 371,
  mf_Delphi = 372,
  mf_Electronic_Design_Inc = 373,
  mf_Northern_Lights_Inc = 374,
  mf_Williams_Controls_Inc = 375,
  mf_Quake_Global = 376,
  mf_ifm_electronic_gmbh = 377,
  mf_Glendinning_Marine_Products = 378,
  mf_Yamabiko_Corporation = 379,
  mf_Suntech_International_Ltd = 380,
  mf_B_and_G = 381,
  mf_National_Agriculture_and_Food_Research_Organization = 382,
  mf_MCL_Industries = 383,
  mf_Camano_Light = 384,
  mf_Johnson_Outdoor_Marine_Electronics = 385,
  mf_JLG_Automation_BVBA = 386,
  mf_Orscheln_Products_LLC = 387,
  mf_Innomatix_LLC = 388,
  mf_Benchmark_Electronics_Minnesota_Division = 389,
  mf_Partech_Inc = 390,
  mf_Electronic_Design_for_Industry_Inc = 391,
  mf_Tianyuan_Technology_Co_Ltd = 392,
  mf_Harvest_Tec_Inc = 393,
  mf_Capi_2_Nederland_BV = 394,
  mf_GENTEC_SRL = 395,
  mf_Beyond_Measure = 396,
  mf_Sanyo_kiki_Co_Ltd = 397,
  mf_Hilite_International = 398,
  mf_ISEKI_and_Co_Ltd = 399,
  mf_Livorsi_Marine = 400,
  mf_Torqeedo_GmbH = 401,
  mf_Simma_Software_Inc = 402,
  mf_Trackwell_ADS_Inc = 403,
  mf_Com_Nav_Marine_Ltd = 404,
  mf_Wema_System_AS = 405,
  mf_Vecima_Networks_Inc = 406,
  mf_Comtech_Mobile_Datacom = 407,
  mf_Corvus_Energy_Ltd = 408,
  mf_Transfluid_SrL = 409,
  mf_COBO_SpA_Divisione_3B6 = 410,
  mf_Hy_Drive_Technologies_Ltd = 411,
  mf_WebTech_Wireless_Inc = 412,
  mf_Datapross_Nijbroek_bv = 413,
  mf_Cattron_Group_International = 414,
  mf_Valid_Manufacturing_Ltd = 415,
  mf_Kubota_Corporation = 416,
  mf_KZValve = 417,
  mf_Intellistick_Inc = 418,
  mf_Fusion_Electronics_Ltd = 419,
  mf_Vermeer_Corporation_ACS_Group = 420,
  mf_Vertex_Standard_Co_Ltd = 421,
  mf_True_Heading_AB = 422,
  mf_BSM_Wireless_Inc = 423,
  mf_Odyne_LLC = 424,
  mf_Methode_Electronics_Inc_MDI = 425,
  mf_Rota_Engineering_Ltd = 429,
  mf_Auteq_Telematica_SA = 430,
  mf_Tohatsu_Corporation = 431,
  mf_S_A_Systems_Inc = 432,
  mf_Rowe_Electronics = 433,
  mf_Stored_Energy_Systems = 434,
  mf_Zunhammer_GmbH = 435,
  mf_Kinze_Manufacturing = 436,
  mf_Digital_Yacht_Limited = 437,
  mf_Comar_Systems_Ltd = 438,
  mf_Hyundai_Heavy_Industries = 439,
  mf_Cummins_Power_Generation = 440,
  mf_PTG_Reifendruckregelsysteme_GmbH = 441,
  mf_Horsch_Maschinen_GmbH = 442,
  mf_SignalQuest_Inc = 443,
  mf_ITT_Power_Solutions = 444,
  mf_KAT_MECHATRONIC_Electronic_Product_Division = 445,
  mf_CertTech_LLC = 446,
  mf_Great_Plains_Mfg = 447,
  mf_Stanadyne_Corporation_Electronics_Systems = 448,
  mf_Polaris_Industries_Inc = 449,
  mf_Dycor_Technologies_Ltd = 450,
  mf_Parker_Hannifin_Corp = 451,
  mf_WIKA_Alexander_Wiegand_SE_and_Co_KG = 452,
  mf_Cooper_Bussmann = 453,
  mf_NGK_Spark_Plug_Co_Ltd = 454,
  mf_ADZ_NAGANO_GmbH = 455,
  mf_General_Kinetics = 456,
  mf_RUSELPROM_ElectricDrive_Ltd = 457,
  mf_Control_Solutions_Inc = 458,
  mf_Alltek_Marine_Electronics_Corp = 459,
  mf_San_Giorgio_SEIN = 460,
  mf_HAWE_Hydraulik_SE = 461,
  mf_IHI_Shibaura_Machinery_Corporation = 462,
  mf_PROBOTIQ = 463,
  mf_Leach_International_Corporation = 464,
  mf_Ashcroft_Inc = 465,
  mf_Veethree_Electronics_and_Marine_LLC = 466,
  mf_Lely_Industries_NV = 467,
  mf_Tyco_Fire_Protection_Products = 468,
  mf_RA_Consulting_GmbH = 469,
  mf_SI_TEX_Marine_Electronics = 470,
  mf_Sea_Cross_Marine_AB = 471,
  mf_Tenneco_Inc = 472,
  mf_Boss_Industries_Inc = 473,
  mf_Persen_Technologies_Inc = 474,
  mf_GME = 475,
  mf_Hummingbird_Marine_Electronics = 476,
  mf_OilQuick_AB = 477,
  mf_OceanSat_BV = 478,
  mf_Vapor_Bus_International = 479,
  mf_EnerDel_Inc = 480,
  mf_Chetco_Digital_Instruments = 481,
  mf_Tricon_Electronics = 482,
  mf_Valeo = 483,
  mf_Headsight_Inc = 484,
  mf_MATT_automotive = 485,
  mf_Westport_Innovations_Inc = 486,
  mf_DSE_Test_Solutions_A_S = 487,
  mf_The_Charles_Machine_Works_Inc = 488,
  mf_Appareo_Systems_LLC = 489,
  mf_QuikQ = 490,
  mf_Penny_and_Giles_Ltd = 491,
  mf_Inergy_Automotive_Systems = 492,
  mf_Watcheye = 493,
  mf_Synerject = 494,
  mf_HOLMER_Maschinenbau_GmbH = 495,
  mf_W_Gessmann_GmbH = 496,
  mf_SENTRON_Sistemas_Embarcados = 497,
  mf_Innovative_Design_Solutions_Inc = 498,
  mf_LCJ_Capteurs = 499,
  mf_Oxbo_International_Corporation = 500,
  mf_Agrotronix_SA = 501,
  mf_Attwood_Corporation = 502,
  mf_Naviop_SRL = 503,
  mf_Vesper_Marine = 504,
  mf_Yetter_Farm_Equipment = 505,
  mf_IHI_STAR_Machinery_Corporation = 506,
  mf_ISOBUS_Test_Center = 507,
  mf_Transtech_Innovations = 508,
  mf_MOTORTECH_GmbH = 509,
  mf_Marinesoft_Co_Ltd = 510,
  mf_Sulky = 511,
  mf_Inpower_LLC = 512,
  mf_Precision_Technology = 513,
  mf_DISTek_Integration_Inc = 514,
  mf_GINAF_Trucks_Nederland_BV = 515,
  mf_AVAT_Automation_GmbH = 516,
  mf_Noland_Engineering = 517,
  mf_Transas_USA_Inc = 518,
  mf_Peeters_Landbouwmachines_bv = 519,
  mf_Trapeze = 520,
  mf_Clever_Devices_Ltd = 521,
  mf_Nebraska_Tractor_Test_Laboratory = 522,
  mf_Reggio_Emilia_Innovazione = 523,
  mf_Vomax_Instrumentation_Pty_Ltd = 524,
  mf_Rust_Sales_INC = 525,
  mf_LOFA_Industries_Inc = 526,
  mf_GKN_Walterscheid_GmbH = 527,
  mf_Hoganas_AB_Electric_Drive_Systems = 528,
  mf_National_Instruments_Korea = 529,
  mf_NMEA = 530,
  mf_Genge_and_Thoma_AG_2 = 531,
  mf_Onwa_Marine_Electronics_Co_Ltd = 532,
  mf_Doran_Manufacturing_LLC = 533,
  mf_Webasto_Thermo_and_Comfort_SE = 534,
  mf_MOTORPAL_as = 535,
  mf_SSI_Technologies = 536,
  mf_Schrader_Electronics_Ltd = 537,
  mf_Crop_Ventures_Inc = 538,
  mf_Mobileview = 539,
  mf_Dinex_A_S = 540,
  mf_Total_Fire_Systems_Inc = 541,
  mf_Dinamica_Generale_spa = 542,
  mf_BAUER_Maschinen_GmbH = 543,
  mf_Au_Group_Electronics = 544,
  mf_GS_Hydraulics = 545,
  mf_Maruyama_Mfg_Co_Inc = 546,
  mf_Thomson_Linear_LLC = 547,
  mf_TM4_Inc = 548,
  mf_ROAD_Deutschland_GmbH = 549,
  mf_SUN_A_Corporation = 550,
  mf_Wexler_CSD_Ltd = 551,
  mf_Matsuyama_Plow_Mfg_Co_Ltd = 552,
  mf_KIB_Electronics = 553,
  mf_iris_GmbH_infrared_and_intelligent_sensors = 554,
  mf_Sasaki_Corporation = 555,
  mf_Doosan_Infracore_Norway = 556,
  mf_Rockson_Automation_GmbH = 557,
  mf_Davis_Instruments_Corp = 558,
  mf_Four_Peaks_Navigation = 559,
  mf_Iowa_State_University_Agricultural_and_Biosystems_Engineering = 560,
  mf_b_plus_GmbH = 561,
  mf_Bombardier_Transportation_GmbH = 562,
  mf_LOHR_Sistemas_Eletronicos_LTDA = 563,
  mf_Auto_Power_Electronic = 564,
  mf_Micro_Trak_Systems_Inc = 565,
  mf_Geode_Technology_Inc = 566,
  mf_Lithiumstart_LLC = 567,
  mf_Makersan_Ltd_Co = 568,
  mf_LORD_MicroStrain_Sensing_Systems = 569,
  mf_frenzel_berg_electronic_GmbH_and_Co_KG = 570,
  mf_Marinecraft_Co_Ltd = 571,
  mf_Fasse_Valves = 572,
  mf_Orolia_Ltd = 573,
  mf_Vishay_Precision_Group = 574,
  mf_Lytx = 575,
  mf_Vectia = 576,
  mf_Denchi_Power_Ltd_ = 577,
  mf_advanSea = 578,
  mf_KVH_Industries_Inc_2 = 579,
  mf_San_Jose_Technology_Inc = 580,
  mf_Vaderstad_Verken_AB = 581,
  mf_Innovative_Software_Engineering = 582,
  mf_Yachtcontrol = 583,
  mf_CarMedialab_GmbH = 584,
  mf_Industrial_Electronic_Controls = 585,
  mf_Suzuki_Motor_Corp = 586,
  mf_JCA_Electronics = 587,
  mf_Vignal_Systems = 588,
  mf_MICO_Inc = 589,
  mf_ARGO_HYTOS_GMBH = 590,
  mf_United_States_Coast_Guard = 591,
  mf_tecsis_GmbH = 592,
  mf_Sensata_Technologies = 593,
  mf_Kongsberg_Automotive = 594,
  mf_CustomWare = 595,
  mf_Brunelco_Electronic_Innovators_BV = 596,
  mf_Hydac_Filter_Systems_GmbH = 597,
  mf_ABB_Turbo_Systems_Ltd = 598,
  mf_Spudnik_Equipment_Co_LLC = 599,
  mf_Aquatic_AV = 600,
  mf_Navitas_Systems = 601,
  mf_Nomad_Digital_Ltd = 602,
  mf_Kereval = 603,
  mf_Rototilt_Group_AB = 604,
  mf_Aventics_GmbH = 605,
  mf_Intellian_Technologies = 606,
  mf_Knappco_Civacon = 607,
  mf_Gale_Banks_Engineering = 608,
  mf_Walvoil_SpA = 609,
  mf_Trail_Tech = 610,
  mf_Esterline = 611,
  mf_Samwon_IT = 612,
  mf_HKS_Co_Ltd = 613,
  mf_ARLT_Technologies_GmbH = 614,
  mf_Networkfleet_Verizon_Telematics = 615,
  mf_SMART_TEC_sro = 616,
  mf_Zero_Emission_Vehicles = 617,
  mf_Evrard_SA = 618,
  mf_Right_Weigh_Load_Scales = 619,
  mf_Sevcon_Ltd = 620,
  mf_Hagie_Manufacturing_Company = 621,
  mf_Floyd_Bell_Inc = 622,
  mf_Xirgo_Technologies = 623,
  mf_Blackbox_Machine_Control_Pty_Ltd = 624,
  mf_Global_MRV_Inc = 625,
  mf_AVL_DiTEST_GmbH = 626,
  mf_Radio_Ocean = 627,
  mf_Falck_Schmidt_Defence_Systems = 628,
  mf_Agri_Info_Design_Ltd = 629,
  mf_SmartDrive_Systems_Inc = 630,
  mf_Reltima = 631,
  mf_Pepperl_Fuchs_GmbH = 632,
  mf_TORC_Robotics = 633,
  mf_Rocky_Research = 634,
  mf_Argo_Tractors_SpA = 635,
  mf_Divelbiss_Corporation = 636,
  mf_Bavaria_Yachtbau_GmbH = 637,
  mf_KVH_Industries_Inc_1 = 638,
  mf_Startec_srl = 639,
  mf_Power_Solutions_International = 640,
  mf_Diverse_Yacht_Services = 641,
  mf_Moog_Aspen_Motion_Technologies = 642,
  mf_Bogballe_A_S = 643,
  mf_KUS_USA = 644,
  mf_esd_electronic_system_design_gmbh = 645,
  mf_Veenhuis_Machines_BV = 646,
  mf_Siloking = 647,
  mf_OJSC_Ekran = 648,
  mf_Control_Q_BV = 649,
  mf_Seiko_Epson_Corp = 650,
  mf_Takakita_Co_Ltd = 651,
  mf_MicroControl_GmbH_and_Co_KG = 652,
  mf_AEV_spol_s_r_o = 653,
  mf_Kohler_Power_Systems_Detroit_Engine_Development_Center = 654,
  mf_Genge_and_Thoma_AG_1 = 655,
  mf_PRO_SOLUS_do_Brasil = 656,
  mf_Terzo_Power_Systems = 657,
  mf_Shenzhen_Jiuzhou_Himunication_Technology_Co_Ltd = 658,
  mf_Data_Panel_Corp_2 = 659,
  mf_Auto_Gaz_Centrum = 660,
  mf_SPAL_Automotive_Srl = 661,
  mf_Kissling_Elektrotechnik_GmbH = 662,
  mf_Delta_Systems_Inc = 663,
  mf_Level_Developments_Ltd = 664,
  mf_Gebr_Bode_GmbH_and_Co_KG = 665,
  mf_Schaeffler_Technologies_AG_and_Co_KG = 666,
  mf_Bartec = 667,
  mf_MacDon_Industries_Ltd = 668,
  mf_Quantum_Fuel_Systems_Technologies_Worldwide_Inc = 669,
  mf_STEMCO_LP = 670,
  mf_Innovative_Controls_Inc = 671,
  mf_OPTIMA_Concept = 672,
  mf_Caruelle_Nicolas = 673,
  mf_Yara_International_ASA = 674,
  mf_Kawasaki_Motors_Corp_USA = 675,
  mf_Danfoss_IXA_A_S = 676,
  mf_DSA_Daten_und_Systemtechnik_GmbH = 677,
  mf_KeepTruckin_Inc = 678,
  mf_OXE_Marine_AB = 679,
  mf_AMVAC_Chemical_Corporation = 680,
  mf_BEDIA_Motorentechnik_GmbH_and_Co_KG = 681,
  mf_Eckelmann_AG = 682,
  mf_Fosen_Elektro = 683,
  mf_KEB = 684,
  mf_ANEDO = 685,
  mf_Taigene_Electric_Machinery_Corp = 686,
  mf_Flight_Systems_Inc = 687,
  mf_Rockford_Corp = 688,
  mf_Aarcomm_Systems_Inc = 689,
  mf_LINAK_A_S = 690,
  mf_Digitroll_Agricultural_Electronics = 691,
  mf_Tanhay_Corporation = 692,
  mf_Agility_Fuel_Solutions = 693,
  mf_GasTOPS_Ltd = 694,
  mf_Weldon_Technologies = 695,
  mf_DRS_Network_and_Imaging_Systems_LLC = 696,
  mf_Alo_AB = 697,
  mf_Scorpion_Technologies_Ltd = 698,
  mf_Harman_International_Industries = 699,
  mf_K_Tec_Earthmovers_Inc = 700,
  mf_BigRoad_Inc = 701,
  mf_Weichai_Power_Co_Ltd = 702,
  mf_Hydro_Tab_Marine_Engineering = 703,
  mf_JL_Audio_Inc = 704,
  mf_SVAB_Hydraulik_AB = 705,
  mf_Shanghai_Diesel_Engine_Corporation_Limited = 706,
  mf_Rochester_Gauges_Inc = 707,
  mf_Lars_Thrane_A_S = 708,
  mf_Marquardt_GmbH = 709,
  mf_Greentronics_Ltd = 710,
  mf_DAS_Co_LTD = 711,
  mf_LOR_Manufacturing_Company_Inc_2 = 712,
  mf_US_Hybrid_Corporation = 713,
  mf_Kobashi_Kogyo_Co_Ltd = 714,
  mf_Autonnic_Research_Ltd = 715,
  mf_Eaton_Control_and_Power_Conversion_Division = 716,
  mf_Yacht_Devices_Ltd = 717,
  mf_Micronet_Inc = 718,
  mf_Cojali_S_L = 719,
  mf_WITZ_Corporation = 720,
  mf_Hypro = 721,
  mf_Contelec_AG = 722,
  mf_Jabil_Inc = 723,
  mf_Electronic_Applications_Inc = 724,
  mf_Hitachi_Construction_Machinery_Co_Ltd = 725,
  mf_MIDORI_PRECISIONS = 726,
  mf_Dhoot_Transmission_Pvt_Ltd = 727,
  mf_Streumaster = 728,
  mf_Liebherr = 729,
  mf_BERTHOUD_AGRICOLE = 730,
  mf_Modine_Manufacturing_Company = 731,
  mf_Gefran_SpA = 732,
  mf_Intendia_SL = 733,
  mf_REAPsystems = 734,
  mf_AEM_Performance_Electronics = 735,
  mf_Terex_Aerial_Work_Platforms_Genie = 736,
  mf_Balluff_GmbH = 737,
  mf_Blue_Ink_Technologies = 738,
  mf_LXNAV_doo = 739,
  mf_e_Traction = 740,
  mf_Carling_Technologies = 741,
  mf_EROAD = 742,
  mf_Daemyung_Elevator_Co_Ltd = 743,
  mf_Woosung_Engineering_Co_Ltd = 744,
  mf_Cidra_Corporate_Services_Inc = 745,
  mf_SureFire_Ag_Systems_Inc = 746,
  mf_Agratronix = 747,
  mf_ISOTTA = 748,
  mf_Chart_Inc = 749,
  mf_Joskin_SA = 750,
  mf_Pacific_Track = 751,
  mf_Deep_Sea_Electronics_Plc = 752,
  mf_AIROD_Technologies = 753,
  mf_Parker_Hannifin_Corp_Automation_Group = 754,
  mf_Firefly_Integrations = 755,
  mf_Maschio_Gaspardo_SPA = 756,
  mf_IMPCO_Technologies = 757,
  mf_Banner_Engineering_Corp = 758,
  mf_Hydro_Gear = 759,
  mf_Bernecker_Rainer_Industrie_Elektronik_GmbH = 760,
  mf_MITA_OLEODINAMICA_SpA = 761,
  mf_ROJ_srl = 762,
  mf_AT_Systems_BVBA = 763,
  mf_Bednar_FMT_sro = 764,
  mf_GIGAVAC = 765,
  mf_Epec_Oy = 766,
  mf_Alliance_Wireless_Technologies_Inc = 767,
  mf_Flores_Automation_LLC = 768,
  mf_Trombetta = 769,
  mf_MONOSEM = 770,
  mf_Shaw_Development_LLC = 771,
  mf_Blink_Marine = 772,
  mf_Clarion_Corporation_of_America = 773,
  mf_Taisho_Corporation = 774,
  mf_ZIEHL_ABEGG_Automotive_GmbH = 775,
  mf_HMI_Systems_LLC = 776,
  mf_Ocean_Signal_Ltd = 777,
  mf_Seakeeper_Inc = 778,
  mf_RLC_Electronic_Systems = 779,
  mf_AVID_Technology_Ltd = 780,
  mf_Poly_Planar_LLC = 781,
  mf_AVR_bv = 782,
  mf_Loup_Electronics_Inc = 783,
  mf_CM_Automotive_Systems_Inc = 784,
  mf_Fischer_Panda_GmbH = 785,
  mf_Johnson_Matthey_Battery_Systems = 786,
  mf_Abertax_Technologies_Limited = 787,
  mf_MoTeC_Pty_Ltd = 788,
  mf_GRADALL = 789,
  mf_VACALL = 790,
  mf_AUTEC = 791,
  mf_Kostal_Mexicana = 792,
  mf_James_Fisher_Prolec = 793,
  mf_Shihlin_Electric_and_Engineering_Corporation = 794,
  mf_Broyda_Enterprises_Pty_Ltd = 795,
  mf_Canadian_Automotive_Industries_Ltd = 796,
  mf_Tides_Marine = 797,
  mf_Lumishore_Ltd = 798,
  mf_Stillwater_Designs_and_Audio_Inc_KICKER = 799,
  mf_Delta_Q_Technologies = 800,
  mf_LOR_Manufacturing_Company_Inc_1 = 801,
  mf_SPBI = 802,
  mf_Gill_Sensors_and_Controls_Limited = 803,
  mf_ASA_Electronics_2 = 804,
  mf_Gundersen_and_Loken_AS = 805,
  mf_Charge_Automotive_Ltd = 806,
  mf_Schneider_Electric = 807,
  mf_RIMEX_Supply_Ltd = 808,
  mf_HMS_Industrial_Networks_AB = 809,
  mf_Dutch_Power_Company = 810,
  mf_Blue_Water_Desalination = 811,
  mf_Torch_Technologies = 812,
  mf_Thales_Suisse_SA = 813,
  mf_Kronotech_Srl = 814,
  mf_FLIR_Systems_Inc = 815,
  mf_UniStrong = 816,
  mf_TE_Connectivity_Sensor_Solutions = 817,
  mf_HGNSS = 818,
  mf_Preco_Electronics = 819,
  mf_DIaLOGIKa_Gesellschaft_fuer_angewandte_Informatik_mbH = 820,
  mf_Thorsen_Teknik_A_S = 821,
  mf_Bren_Tronics_Inc = 822,
  mf_ACEINNA_Inc = 823,
  mf_Undheim_Systems_AS = 824,
  mf_BPW_Hungaria = 825,
  mf_Lewmar_Marine_Inc = 826,
  mf_INmatix_Technology_Group_Ltd = 827,
  mf_AgriBrink_Inc = 828,
  mf_Drov_Technologies_Inc = 829,
  mf_Ultra_Motion_LLC = 830,
  mf_DOK_ING_Ltd = 831,
  mf_Heizomat_Geratebau_Energiesysteme_GmbH = 832,
  mf_Electrum_Automation_AB = 833,
  mf_Pioneer_Microsystems_Inc = 834,
  mf_PetTrack_Ltd = 835,
  mf_Signature4 = 836,
  mf_Famic_Technologies_Inc = 837,
  mf_Teamsurv_Ltd = 838,
  mf_Indexator_Rotator_Systems_AB = 839,
  mf_ESTE_srl = 840,
  mf_Agra_GPS = 841,
  mf_DigiDevice_Srl = 842,
  mf_Hendrickson_Truck_Commercial_Vehicle_Systems = 843,
  mf_FELL_AS = 844,
  mf_GMB_Gustrower_Maschinenbau_GmbH = 845,
  mf_L3_Magnet_Motor = 846,
  mf_Oceanvolt = 847,
  mf_ningupex = 848,
  mf_MTS_Maschinentechnik_Schrode_AG = 849,
  mf_Geoprospectors_GmbH = 850,
  mf_Novotechnik_Messwertaufnehmer_OHG = 851,
  mf_Velvac_Inc = 852,
  mf_Teledyne_RESON_BV = 853,
  mf_GEMAC_Chemnitz_GmbH = 854,
  mf_Toshiba_Infrastructure_Systems_and_Solutions_Corporation = 855,
  mf_FarmFacts_GmbH = 856,
  mf_Zoomlion_Heavy_Industry_NA_Inc = 857,
  mf_Littelfuse = 858,
  mf_Valor = 859,
  mf_Michelin = 860,
  mf_Adel_System_Srl = 861,
  mf_Prospec_Electronics = 862,
  mf_SMAG = 863,
  mf_Streamline_Transportation_Technologies_Inc = 864,
  mf_MyEasyFarm = 865,
  mf_Netradyne_Inc = 866,
  mf_Skeleton_Technologies = 867,
  mf_Data_Panel_Corp_1 = 868,
  mf_Intercomp = 869,
  mf_Textron_Fleet_Management = 870,
  mf_Superior_Tech_Inc = 871,
  mf_Kahlig_Antriebstechnik_GmbH = 872,
  mf_Garnet_Instruments_Ltd = 873,
  mf_MTA_SpA = 874,
  mf_Salvarani_Srl = 875,
  mf_SUCO_Robert_Scheuffele_GmbH_and_Co_KG = 876,
  mf_SICK_ATech_GmbH = 877,
  mf_FOTON = 878,
  mf_PG_Trionic_Inc = 879,
  mf_Revision_Military = 880,
  mf_Sovema = 881,
  mf_BHTronik_GmbH_and_Co_KG = 882,
  mf_Swift_Navigation_Inc = 883,
  mf_Sure_Grip_Controls_Inc = 884,
  mf_SICK_Stegmann_GmbH = 885,
  mf_Hitachi_Zosen_Corporation = 886,
  mf_Hyperdrive_Innovation_Limited = 887,
  mf_Carma_Systems_Inc = 888,
  mf_SANY_America_Inc = 889,
  mf_L3_Technologies_Inc = 890,
  mf_CarrierWeb = 891,
  mf_Mectronx_Corp = 892,
  mf_KlinkTechnics_Ltd = 893,
  mf_Rhodan_Marine_Systems_of_Florida_LLC_1 = 894,
  mf_Peloton_Technology = 895,
  mf_NextFour_Solutions_Ltd = 896,
  mf_Dot_Technology_Corp = 897,
  mf_Josef_Kotte_Landtechnik_GmbH_and_Co_KG = 898,
  mf_Lippert_Components_Inc = 899,
  mf_OSB_AG = 900,
  mf_Zivan_Srl = 901,
  mf_Bucher_Hydraulics = 902,
  mf_InMotion = 903,
  mf_Equipment_Safety_Systems_Pty_Ltd = 904,
  mf_ASA_Electronics_1 = 905,
  mf_MOBA_AG = 906,
  mf_strautmann = 907,
  mf_Chonbuk_National_University_Department_of_Electronics = 908,
  mf_Marines_Co_Ltd = 909,
  mf_Hermann_Paus_Maschinenfabrik_GmbH = 910,
  mf_Nautic_On = 911,
  mf_Steyr_Motors_GmbH = 912,
  mf_Toyota_Motor_Corporation = 913,
  mf_ZETOR_TRACTORS_as = 914,
  mf_Dosificacion_y_sistemas_electronicos_SL = 915,
  mf_Turck_Inc = 916,
  mf_Sentinel_doo = 917,
  mf_AMW_Machine_Control_Inc = 918,
  mf_Furrion_LLC = 919,
  mf_MTD_Consumer_Group_Inc = 920,
  mf_ROPA_Fahrzeug_und_Maschinenbau_GmbH = 921,
  mf_Terberg_Benschop_BV = 922,
  mf_ELTEK_SpA = 923,
  mf_PAS_Peschak_Autonome_Systeme_GmbH = 924,
  mf_GW_Lisk_Company = 925,
  mf_Buhler_Industries_Inc = 926,
  mf_Saicon = 927,
  mf_EAO_Automotive_GmbH_and_Co_KG = 928,
  mf_Jl_Marine_Systems_Inc = 929,
  mf_Ecotronix_Corp = 930,
  mf_Enertec_Marine_Ltd = 931,
  mf_Discover_Battery = 932,
  mf_Delta_Mobile_Systems_Inc = 933,
  mf_Farmobile_Inc = 934,
  mf_Meels_GmbH_and_Co_KG = 935,
  mf_Hi_tech_Millennium = 936,
  mf_Precision_Circuits_Inc = 937,
  mf_LEVEL_Systems = 938,
  mf_Warn_Industries_Inc = 939,
  mf_Dometal_Oy = 940,
  mf_Navya = 941,
  mf_Vogelsang_GmbH_and_Co_KG = 942,
  mf_FISCHER_AG_Prazisionspindeln = 943,
  mf_ZONTISA_Marine_SL = 944,
  mf_Equalizer_AG = 945,
  mf_Hydac_Tecnologia_Ltda = 946,
  mf_Gerd_Bar_GmbH = 947,
  mf_Donix = 948,
  mf_Skoda_Electric_as = 949,
  mf_RE_Lab_srl = 950,
  mf_Exor_International_SpA = 951,
  mf_Tan_Delta_Systems_Ltd = 952,
  mf_Curtiss_Wright = 953,
  mf_Setec_Pty_Ltd = 954,
  mf_Hottinger_Baldwin_Electronic_Measurement_Technology_Co_Ltd = 955,
  mf_Piher_Sensors_and_Controls_SA = 956,
  mf_Sierra_Wireless_Inc = 957,
  mf_NHK_MEC_Corporation = 958,
  mf_Cascade_Corporation = 959,
  mf_Vemcon_GmbH = 960,
  mf_POK_SAS = 961,
  mf_Timbolier_Industries_Inc = 962,
  mf_Thomason_Jones_Company_LLC = 963,
  mf_AgroVIR_Kft = 964,
  mf_Ro_Sys_Software = 965,
  mf_VisibleFarm = 966,
  mf_Moteck_Electric_Corp = 967,
  mf_Cox_Powertrain_Limited = 968,
  mf_Blue_Sea_Systems = 969,
  mf_Jaboni_Power_Products_LLC = 970,
  mf_B_B_Smartworx = 971,
  mf_Axion_AG = 972,
  mf_Zasso_GmbH = 973,
  mf_Pico_Technology_Limited = 974,
  mf_Siemens_AG = 975,
  mf_Derive_Systems_Inc = 976,
  mf_Emerson_Electric_Co = 977,
  mf_Canfield_Connector = 978,
  mf_McHale = 979,
  mf_Gussi_Italia_SRL = 980,
  mf_Kobelt_Manufacturing_Co_Ltd = 981,
  mf_Briggs_and_Stratton_Corporation = 982,
  mf_Dezwaef_NV = 983,
  mf_E_T_A_Elektrotechnische_Apparate_GmbH = 984,
  mf_NLR_LLC = 985,
  mf_AAMP_Global = 986,
  mf_Baltic_Car_Equipment = 987,
  mf_Voith_Turbo = 988,
  mf_Flux_Gerate_GmbH = 989,
  mf_SAMSUNG_SDI_Co_Ltd = 990,
  mf_Deutronic_Elektronik_GmbH = 991,
  mf_MarineIOT_LLC = 992,
  mf_Alelion_Energy_Systems_AB = 993,
  mf_Morris_Industries_Ltd = 994,
  mf_Circuitlink_Pty_Ltd = 995,
  mf_Eniquest_Pty_Ltd = 996,
  mf_Xenta_Systems_Srl = 997,
  mf_Transcell_Technology_Inc = 998,
  mf_TerraTroniq_BV = 999,
  mf_Landmaschinen_Wienhoff_GmbH = 1000,
  mf_Cleral_Inc = 1001,
  mf_Kussmaul_Electronics_Co = 1002,
  mf_technotrans_SE = 1003,
  mf_Ultraflex_SpA = 1004,
  mf_TSE_Brakes_Inc = 1005,
  mf_Proterra = 1006,
  mf_Maschinenfabrik_Meyer_Lohne_GmbH = 1007,
  mf_Lintest_Systems_LLC = 1008,
  mf_TouchTronics_Inc = 1009,
  mf_Agricultural_Industry_Electronics_Foundation_eV = 1010,
  mf_Soundmax_Electronics_Ltd = 1011,
  mf_Rhodan_Marine_Systems_of_Florida_LLC_2 = 1012,
  mf_Allochis_E_Tec = 1013,
  mf_Briri_Maschinenbau_GmbH = 1014,
  mf_SECURITAG_SAS = 1015,
  mf_Rohren_und_Pumpenwerk_BAUER_GmbH = 1016,
  mf_Caldaro_AB = 1017,
  mf_Flex_TTS = 1018,
  mf_Codek_Foundries = 1019,
  mf_Onyx_Marine_Automation_srl = 1020,
  mf_Entratech_Systems_LLC = 1021,
  mf_ITC_Inc = 1022,
  mf_PEAK_System_Technik_GmbH = 1023,
  mf_Inventus_Power = 1024,
  mf_eze_System_Inc = 1025,
  mf_ZTR_Control_Systems_LLC = 1026,
  mf_Goldacres_PTY_LTD = 1027,
  mf_Autel_Intelligent_Technology_Corp_Ltd = 1028,
  mf_The_Marine_Guardian_LLC = 1029,
  mf_Tume_Agri_Oy = 1030,
  mf_Flintec_UK_Ltd = 1031,
  mf_SIMOL_SpA = 1032,
  mf_ECCO_Safety_Group = 1033,
  mf_Siren_Marine = 1034,
  mf_Agres = 1035,
  mf_Bender_GmbH_and_Co_KG = 1036,
  mf_Dragonfly_Energy_Corp = 1037,
  mf_EMIT_Technologies_Inc = 1038,
  mf_Liugong_Dressta_Machinery = 1039,
  mf_BPE_Electronics_Srl = 1040,
  mf_EZ_Lynk = 1041,
  mf_engcon_group = 1042,
  mf_Hyva = 1043,
  mf_Xee = 1044,
  mf_CMR_Group = 1045,
  mf_Praxidyn = 1046,
  mf_Sonic_Corporation = 1047,
  mf_Agrivation_UG = 1048,
  mf_S_S_Cycle = 1049,
  mf_Spacenus_GmbH = 1050,
  mf_ProNav_AS = 1051,
  mf_Pfreundt = 1052,
  mf_Vetus_Maxwell_Inc = 1053,
  mf_Schmotzer_Hacktechnik_GmbH_and_Co_KG = 1054,
  mf_EXA_Computing_GmbH = 1055,
  mf_Lithium_Pros = 1056,
  mf_EquipmentShare = 1057,
  mf_Bondioli_and_Pavesi_SpA = 1058,
  mf_Boatrax = 1059,
  mf_Scale_Tec = 1060,
  mf_Volvo_Construction_Equipment = 1061,
  mf_Marol_Co_Ltd = 1062,
  mf_SolSteer = 1063,
  mf_Omnitech_Robotics_Inc = 1064,
  mf_CALYPSO_Instruments = 1065,
  mf_Spot_Zero_Water = 1066,
  mf_Tecomec_Srl = 1067,
  mf_Zapi_SpA = 1068,
  mf_Lithionics_Battery_LLC = 1069,
  mf_Quick_teck_Electronics_Ltd = 1070,
  mf_MACH_SYSTEMS_sro = 1071,
  mf_La_Marche_Manufacturing_Company = 1072,
  mf_Manitou_Equipment_America_LLC = 1073,
  mf_Kindhelm_Navigs_Oy = 1074,
  mf_Uniden_America_Corporation = 1075,
  mf_DOGA_SA = 1076,
  mf_Ymer_Technology = 1077,
  mf_GE_Appliances = 1078,
  mf_Carl_Zeiss_Spectroscopy_GmbH = 1079,
  mf_Epsilor_Electric_Fuel_Ltd = 1080,
  mf_Imcon_Electronics_sro = 1081,
  mf_TUMOSAN_Engine_and_Tractor_Co = 1082,
  mf_Nauticoncept = 1083,
  mf_Shadow_Caster_LED_lighting_LLC = 1084,
  mf_Wet_Sounds_LLC = 1085,
  mf_Life_Racing_Ltd = 1086,
  mf_ITPhotonics_Srl = 1087,
  mf_E_T_A_Circuit_Breakers = 1088,
  mf_Carrosserie_HESS_AG = 1089,
  mf_E_T_A_Circuit_Breakers_Ltd = 1090,
  mf_Wheel_Monitor_Inc = 1091,
  mf_Scheiber = 1092,
  mf_innolectric = 1093,
  mf_Tokushu_Denki = 1094,
  mf_WEG_Drives_and_Controls_Automacao_Ltda = 1095,
  mf_Texense_Sensors = 1096,
  mf_M2M_Craft_Co_Ltd = 1097,
  mf_Preh_GmbH = 1098,
  mf_MadgeTech_Inc = 1099,
  mf_Smart_Yachts_International_Limited = 1100,
  mf_DIS_Sensors_BV = 1101,
  mf_Carl_Geringhoff_GmbH_and_Co_KG = 1102,
  mf_Diesel_Laptops = 1103,
  mf_The_Toro_Company = 1104,
  mf_Redekop_Manufacturing = 1105,
  mf_SEPPI_M = 1111,
  mf__621_Technologies_Inc = 1569,
  mf_Seastar_Solutions = 1850,
  mf_RayMarine = 1851,
  mf_Navionics = 1852,
  mf_Japan_Radio_Co = 1853,
  mf_Northstar_Technologies = 1854,
  mf_Furuno_USA = 1855,
  mf_Trimble = 1856,
  mf_Simrad = 1857,
  mf_Litton = 1858,
  mf_Kvasar_AB = 1859,
  mf_MMP = 1860,
  mf_Vector_North_America = 1861,
  mf_Sanshin = 1862,
  mf_Thomas_G_Faria_Co = 1863,
};

/**
 * \enum CanMessageConfirmation
 *
 */
enum CanMessageConfirmation
{
  eMessageSent,
  eMessageFailed
};


#define PGN_ProprietaryB(x)                 PGN_ProprietaryB_start + ((x) & 0xFF)


// Timeouts
// Maximum time for a message to be in "Wait For COnfirmation" state
#define CAN_MESSAGE_MAX_CONFIRMATION_TIME   (1000) // 1s

// Time to wait after request for address claimed sent
#define CAN_ADDRESS_CLAIMED_WAITING_TIME    (250) 


} // can
} // brt

