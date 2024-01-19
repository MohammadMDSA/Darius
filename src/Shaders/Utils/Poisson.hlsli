/*
Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
*/


// samples = 8, min distance = 0.5, average samples on radius = 2
static const float3 gPoisson8[8] =
{
    float3(-0.4706069, -0.4427112, +0.6461146),
    float3(-0.9057375, +0.3003471, +0.9542373),
    float3(-0.3487388, +0.4037880, +0.5335386),
    float3(+0.1023042, +0.6439373, +0.6520134),
    float3(+0.5699277, +0.3513750, +0.6695386),
    float3(+0.2939128, -0.1131226, +0.3149309),
    float3(+0.7836658, -0.4208784, +0.8895339),
    float3(+0.1564120, -0.8198990, +0.8346850)
};

// samples = 16, min distance = 0.38, average samples on radius = 2
static const float3 gPoisson16[16] =
{
    float3(-0.0936476, -0.7899283, +0.7954600),
    float3(-0.1209752, -0.2627860, +0.2892948),
    float3(-0.5646901, -0.7059856, +0.9040413),
    float3(-0.8277994, -0.1538168, +0.8419688),
    float3(-0.4620740, +0.1951437, +0.5015910),
    float3(-0.7517998, +0.5998214, +0.9617633),
    float3(-0.0812514, +0.2904110, +0.3015631),
    float3(-0.2397440, +0.7581663, +0.7951688),
    float3(+0.2446934, +0.9202285, +0.9522055),
    float3(+0.4943011, +0.5736654, +0.7572486),
    float3(+0.3415412, +0.1412707, +0.3696049),
    float3(+0.8744238, +0.3246290, +0.9327384),
    float3(+0.7406740, -0.1434729, +0.7544418),
    float3(+0.3658852, -0.3596551, +0.5130534),
    float3(+0.7880974, -0.5802425, +0.9786618),
    float3(+0.3776688, -0.7620423, +0.8504953)
};

// samples = 32, min distance = 0.26, average samples on radius = 3
static const float3 gPoisson32[32] =
{
    float3(-0.1078042, -0.6434212, +0.6523899),
    float3(-0.1141091, -0.9539828, +0.9607830),
    float3(-0.1982531, -0.3867292, +0.4345846),
    float3(-0.5254982, -0.6604451, +0.8440000),
    float3(-0.1820032, -0.0936076, +0.2046645),
    float3(-0.4654744, -0.2629388, +0.5346057),
    float3(-0.7419540, -0.4592809, +0.8726023),
    float3(-0.7180300, -0.1888005, +0.7424370),
    float3(-0.9541028, -0.0789064, +0.9573601),
    float3(-0.6718881, +0.1745270, +0.6941854),
    float3(-0.3968981, +0.1973703, +0.4432642),
    float3(-0.8614085, +0.4183342, +0.9576158),
    float3(-0.5961362, +0.6559430, +0.8863631),
    float3(-0.0866527, +0.2057932, +0.2232925),
    float3(-0.3287578, +0.7094890, +0.7819567),
    float3(-0.0408453, +0.5730602, +0.5745140),
    float3(-0.0678108, +0.8920295, +0.8946033),
    float3(+0.2702191, +0.9020523, +0.9416564),
    float3(+0.2961993, +0.4006296, +0.4982350),
    float3(+0.5824130, +0.7839746, +0.9766376),
    float3(+0.6095408, +0.4801217, +0.7759233),
    float3(+0.5025840, +0.2096348, +0.5445525),
    float3(+0.2740403, +0.0734566, +0.2837146),
    float3(+0.9130731, +0.4032195, +0.9981425),
    float3(+0.7560658, +0.1432026, +0.7695079),
    float3(+0.6737013, -0.1910683, +0.7002717),
    float3(+0.8628370, -0.3914889, +0.9474974),
    float3(+0.7032576, -0.5988359, +0.9236751),
    float3(+0.4578032, -0.4541197, +0.6448321),
    float3(+0.1706552, -0.3115532, +0.3552304),
    float3(+0.2061829, -0.5709705, +0.6070574),
    float3(+0.3269635, -0.9024802, +0.9598832)
};

// samples = 64, min distance = 0.18, average samples on radius = 5
static const float3 gPoisson64[64] =
{
    float3(-0.0065114, -0.1460582, +0.1462033),
    float3(-0.0303039, -0.9686066, +0.9690805),
    float3(-0.1029292, -0.8030527, +0.8096222),
    float3(-0.1531820, -0.6213900, +0.6399924),
    float3(-0.3230599, -0.8868585, +0.9438674),
    float3(-0.1951447, -0.3146919, +0.3702870),
    float3(-0.3462451, -0.6440054, +0.7311831),
    float3(-0.3455329, -0.4411035, +0.5603260),
    float3(-0.6277606, -0.6978221, +0.9386368),
    float3(-0.6238620, -0.4722686, +0.7824586),
    float3(-0.3958989, -0.2521870, +0.4693977),
    float3(-0.8186533, -0.4641639, +0.9410852),
    float3(-0.6481082, -0.2896534, +0.7098897),
    float3(-0.9109314, -0.1374674, +0.9212455),
    float3(-0.6602813, -0.0511829, +0.6622621),
    float3(-0.3327182, -0.0034168, +0.3327357),
    float3(-0.9708222, +0.0864033, +0.9746596),
    float3(-0.7995708, +0.1496022, +0.8134459),
    float3(-0.4509301, +0.1788653, +0.4851090),
    float3(-0.1161801, +0.0573019, +0.1295427),
    float3(-0.6471452, +0.2481229, +0.6930814),
    float3(-0.8052469, +0.4099220, +0.9035810),
    float3(-0.4898830, +0.3552727, +0.6051480),
    float3(-0.6336213, +0.4714487, +0.7897720),
    float3(-0.6885121, +0.7122980, +0.9906651),
    float3(-0.4522108, +0.5375718, +0.7024800),
    float3(-0.1841745, +0.2540318, +0.3137712),
    float3(-0.2724991, +0.5243348, +0.5909169),
    float3(-0.3906980, +0.8645544, +0.9487356),
    float3(-0.1517160, +0.7061030, +0.7222183),
    float3(-0.1148268, +0.9200021, +0.9271403),
    float3(-0.0228051, +0.5112054, +0.5117138),
    float3(+0.0387527, +0.6830538, +0.6841522),
    float3(+0.0556644, +0.3292533, +0.3339255),
    float3(+0.1651443, +0.8762763, +0.8917022),
    float3(+0.3430057, +0.7856857, +0.8572952),
    float3(+0.3516012, +0.5249697, +0.6318359),
    float3(+0.2562977, +0.3190902, +0.4092762),
    float3(+0.5771080, +0.7862252, +0.9752967),
    float3(+0.6529276, +0.6084227, +0.8924643),
    float3(+0.5189329, +0.4425537, +0.6820155),
    float3(+0.8118719, +0.4586847, +0.9324846),
    float3(+0.3119081, +0.1337896, +0.3393911),
    float3(+0.5046800, +0.1606769, +0.5296404),
    float3(+0.6844428, +0.2401899, +0.7253641),
    float3(+0.8718888, +0.2715452, +0.9131960),
    float3(+0.1815740, +0.0086135, +0.1817782),
    float3(+0.9897170, +0.1209020, +0.9970742),
    float3(+0.6336590, +0.0174913, +0.6339004),
    float3(+0.8165796, +0.0200828, +0.8168265),
    float3(+0.4508830, -0.0892848, +0.4596382),
    float3(+0.9695752, -0.1212535, +0.9771277),
    float3(+0.5904603, -0.2048051, +0.6249708),
    float3(+0.7404402, -0.3184013, +0.8059970),
    float3(+0.9107504, -0.3932986, +0.9920434),
    float3(+0.2479053, -0.2340817, +0.3409564),
    float3(+0.7222927, -0.5845174, +0.9291756),
    float3(+0.4767374, -0.4289174, +0.6412867),
    float3(+0.4893593, -0.7637584, +0.9070829),
    float3(+0.2963522, -0.6137760, +0.6815759),
    float3(+0.1755842, -0.4334003, +0.4676170),
    float3(+0.1360411, -0.7557332, +0.7678801),
    float3(+0.1855755, -0.9548430, +0.9727093),
    float3(+0.0002820, -0.5056334, +0.5056335)
};

// NOTE: samples = 96, min distance = 0.15, average samples on radius = 6
static const float3 gPoisson96[96] =
{
    float3(-0.0403876, -0.8419777, +0.8429458),
    float3(-0.0866264, -0.5079851, +0.5153183),
    float3(-0.1224081, -0.9850855, +0.9926617),
    float3(-0.1226595, -0.6816584, +0.6926063),
    float3(-0.1191302, -0.3471802, +0.3670505),
    float3(-0.2397694, -0.8340476, +0.8678277),
    float3(-0.2812804, -0.6782048, +0.7342209),
    float3(-0.2377271, -0.5337023, +0.5842537),
    float3(-0.0793476, -0.1517703, +0.1712608),
    float3(-0.4919034, -0.8653889, +0.9954230),
    float3(-0.4550894, -0.6634924, +0.8045673),
    float3(-0.3381177, -0.4022819, +0.5255039),
    float3(-0.5085003, -0.5066661, +0.7178322),
    float3(-0.6749743, -0.7097090, +0.9794270),
    float3(-0.6723632, -0.4928165, +0.8336309),
    float3(-0.3238158, -0.1970847, +0.3790766),
    float3(-0.5139163, -0.3216180, +0.6062574),
    float3(-0.6831340, -0.2914454, +0.7427062),
    float3(-0.4764391, -0.1735475, +0.5070631),
    float3(-0.8831391, -0.3860794, +0.9638423),
    float3(-0.7554776, -0.1553841, +0.7712916),
    float3(-0.9237850, -0.1836212, +0.9418574),
    float3(-0.5083610, +0.0086067, +0.5084339),
    float3(-0.9567527, -0.0078530, +0.9567850),
    float3(-0.6818218, +0.0244445, +0.6822599),
    float3(-0.2927991, +0.0333949, +0.2946973),
    float3(-0.1420011, +0.0395289, +0.1474003),
    float3(-0.8947619, +0.1483836, +0.9069822),
    float3(-0.7663029, +0.2735212, +0.8136547),
    float3(-0.6029718, +0.2360898, +0.6475441),
    float3(-0.9012361, +0.3643323, +0.9720929),
    float3(-0.4431779, +0.2416853, +0.5047954),
    float3(-0.6167140, +0.4098776, +0.7404970),
    float3(-0.7698247, +0.5252072, +0.9319188),
    float3(-0.4591635, +0.4254926, +0.6259993),
    float3(-0.6193955, +0.5780694, +0.8472397),
    float3(-0.1571103, +0.2054507, +0.2586381),
    float3(-0.4123918, +0.5897211, +0.7196096),
    float3(-0.5237168, +0.7524166, +0.9167388),
    float3(-0.2315706, +0.4110785, +0.4718162),
    float3(-0.4324275, +0.9015638, +0.9999054),
    float3(-0.2602250, +0.7798824, +0.8221518),
    float3(-0.1855088, +0.6405326, +0.6668550),
    float3(-0.0631948, +0.3238317, +0.3299402),
    float3(-0.2361725, +0.9591521, +0.9878007),
    float3(-0.0018598, +0.1074120, +0.1074281),
    float3(-0.0804199, +0.7839980, +0.7881118),
    float3(+0.0137250, +0.5012080, +0.5013959),
    float3(+0.0302112, +0.6611616, +0.6618515),
    float3(+0.0163704, +0.9598445, +0.9599841),
    float3(+0.1857906, +0.9584860, +0.9763266),
    float3(+0.0784874, +0.2417331, +0.2541558),
    float3(+0.1357376, +0.4062127, +0.4282913),
    float3(+0.1845639, +0.5740392, +0.6029800),
    float3(+0.2254979, +0.7750816, +0.8072179),
    float3(+0.3838611, +0.8303300, +0.9147663),
    float3(+0.2958074, +0.4314820, +0.5231431),
    float3(+0.4304548, +0.6814911, +0.8060530),
    float3(+0.5370785, +0.7913437, +0.9563881),
    float3(+0.4443785, +0.5258204, +0.6884470),
    float3(+0.5771415, +0.6401811, +0.8619305),
    float3(+0.3623219, +0.2960911, +0.4679179),
    float3(+0.7255664, +0.6867011, +0.9990020),
    float3(+0.6815006, +0.5108145, +0.8516892),
    float3(+0.8464920, +0.5122826, +0.9894353),
    float3(+0.6020624, +0.2977475, +0.6716641),
    float3(+0.8042987, +0.3536090, +0.8785987),
    float3(+0.2394170, +0.0792043, +0.2521782),
    float3(+0.4519147, +0.1219826, +0.4680883),
    float3(+0.9526030, +0.2988966, +0.9983945),
    float3(+0.7082511, +0.1612283, +0.7263706),
    float3(+0.8462632, +0.0930516, +0.8513636),
    float3(+0.6101166, +0.0365563, +0.6112108),
    float3(+0.9863577, -0.1182441, +0.9934199),
    float3(+0.8190978, -0.1294892, +0.8292699),
    float3(+0.6563655, -0.1232929, +0.6678450),
    float3(+0.2826931, -0.1012181, +0.3002674),
    float3(+0.4911776, -0.1628683, +0.5174761),
    float3(+0.1163677, -0.0484713, +0.1260591),
    float3(+0.8974063, -0.2732542, +0.9380863),
    float3(+0.7553440, -0.3278418, +0.8234226),
    float3(+0.5750262, -0.3089627, +0.6527734),
    float3(+0.8830774, -0.4400037, +0.9866250),
    float3(+0.3707938, -0.2564998, +0.4508660),
    float3(+0.6983998, -0.5076644, +0.8634149),
    float3(+0.4854268, -0.4372651, +0.6533299),
    float3(+0.7143911, -0.6611294, +0.9733688),
    float3(+0.1537492, -0.2076075, +0.2583402),
    float3(+0.2936103, -0.3914900, +0.4893582),
    float3(+0.5420131, -0.7008795, +0.8860081),
    float3(+0.2966139, -0.6110919, +0.6792740),
    float3(+0.3792824, -0.8804409, +0.9586612),
    float3(+0.1106483, -0.3808194, +0.3965684),
    float3(+0.2513747, -0.7631646, +0.8034982),
    float3(+0.1178393, -0.6159950, +0.6271650),
    float3(+0.1382435, -0.9177985, +0.9281516)
};

// NOTE: samples = 128, min distance = 0.13, average samples on radius = 6
static const float3 gPoisson128[128] =
{
    float3(-0.7089940, -0.6214720, +0.9428149),
    float3(-0.5671330, -0.6822230, +0.8871686),
    float3(-0.9786960, -0.1986800, +0.9986589),
    float3(-0.8081850, -0.4610650, +0.9304536),
    float3(-0.9891760, +0.1190640, +0.9963159),
    float3(-0.9409420, +0.2577720, +0.9756117),
    float3(-0.5741980, +0.7546090, +0.9482289),
    float3(-0.7324710, +0.6592870, +0.9854811),
    float3(-0.0525410, -0.8784000, +0.8799700),
    float3(-0.1610590, -0.9578300, +0.9712766),
    float3(-0.3792030, -0.4380960, +0.5794161),
    float3(-0.3822610, -0.3019270, +0.4871174),
    float3(-0.3764430, +0.4197840, +0.5638510),
    float3(-0.4956710, +0.3263450, +0.5934566),
    float3(-0.4039250, +0.8622360, +0.9521587),
    float3(-0.1245920, +0.9874620, +0.9952911),
    float3(+0.0768400, -0.9224200, +0.9256150),
    float3(+0.2310630, -0.9303730, +0.9586366),
    float3(+0.0274670, -0.0147730, +0.0311878),
    float3(+0.2863450, -0.0145380, +0.2867138),
    float3(+0.1018970, +0.3734240, +0.3870768),
    float3(+0.1954070, +0.4803110, +0.5185389),
    float3(+0.0837160, +0.8588590, +0.8629294),
    float3(+0.0853160, +0.5643070, +0.5707199),
    float3(+0.5281740, -0.7293590, +0.9005178),
    float3(+0.6464420, -0.6162280, +0.8930981),
    float3(+0.8951860, -0.4160920, +0.9871629),
    float3(+0.9366010, -0.2850270, +0.9790106),
    float3(+0.6702940, +0.4672220, +0.8170621),
    float3(+0.5202950, +0.3433550, +0.6233776),
    float3(+0.8388290, +0.5418810, +0.9986336),
    float3(+0.5227520, +0.7146860, +0.8854635),
    float3(-0.6728040, -0.4958620, +0.8357896),
    float3(-0.5258120, -0.4882710, +0.7175561),
    float3(-0.6513380, +0.1081620, +0.6602576),
    float3(-0.8251040, +0.3527810, +0.8973578),
    float3(-0.7794940, +0.5090390, +0.9309842),
    float3(-0.5991140, +0.5553780, +0.8169347),
    float3(-0.1685700, -0.8036030, +0.8210930),
    float3(-0.3734380, -0.9071870, +0.9810424),
    float3(-0.1481460, -0.1147090, +0.1873643),
    float3(-0.2116510, -0.2408530, +0.3206342),
    float3(-0.0487150, +0.4884940, +0.4909170),
    float3(-0.3556680, +0.2905100, +0.4592339),
    float3(-0.2024920, +0.8664630, +0.8898096),
    float3(-0.0534200, +0.8531320, +0.8548028),
    float3(+0.2118690, -0.7915620, +0.8194259),
    float3(+0.0788500, -0.7116190, +0.7159742),
    float3(+0.0428120, -0.3753060, +0.3777399),
    float3(+0.1605940, -0.4388590, +0.4673196),
    float3(+0.2080920, +0.2891210, +0.3562208),
    float3(+0.1810880, +0.1535040, +0.2373949),
    float3(+0.4325840, +0.8918800, +0.9912511),
    float3(+0.1999780, +0.9297440, +0.9510074),
    float3(+0.8003100, -0.5964810, +0.9981412),
    float3(+0.5758440, -0.5004610, +0.7629269),
    float3(+0.9621100, -0.1146000, +0.9689111),
    float3(+0.8309570, -0.1460440, +0.8436933),
    float3(+0.5528250, +0.2090580, +0.5910336),
    float3(+0.8132120, +0.3897220, +0.9017743),
    float3(+0.6562940, +0.7456790, +0.9933574),
    float3(+0.5248890, +0.5826980, +0.7842483),
    float3(-0.9209760, -0.3289270, +0.9779518),
    float3(-0.5581920, -0.3306940, +0.6487964),
    float3(-0.8551130, +0.1380860, +0.8661906),
    float3(-0.7648520, +0.0022930, +0.7648554),
    float3(-0.4507560, -0.7735720, +0.8953182),
    float3(-0.3194260, -0.7572700, +0.8218825),
    float3(-0.2036160, -0.3950070, +0.4443985),
    float3(-0.3067120, -0.1341980, +0.3347856),
    float3(-0.2098050, +0.2085110, +0.2957955),
    float3(-0.1692030, +0.3812370, +0.4170987),
    float3(-0.4147870, +0.7063550, +0.8191371),
    float3(-0.2136750, +0.6780910, +0.7109602),
    float3(+0.1286520, -0.5657470, +0.5801905),
    float3(+0.2864410, -0.6815780, +0.7393220),
    float3(+0.2617770, -0.2838630, +0.3861417),
    float3(+0.0149550, -0.1624500, +0.1631369),
    float3(+0.3157310, +0.1210170, +0.3381289),
    float3(+0.4195410, +0.2304200, +0.4786523),
    float3(+0.0156850, +0.9764970, +0.9766230),
    float3(+0.0651120, +0.7204850, +0.7234212),
    float3(+0.6369040, -0.0393900, +0.6381209),
    float3(+0.7926540, -0.2760840, +0.8393585),
    float3(+0.6903180, +0.3282840, +0.7644013),
    float3(+0.7243520, +0.1553030, +0.7408136),
    float3(+0.7288860, +0.6330180, +0.9653945),
    float3(-0.7309940, -0.3456690, +0.8086033),
    float3(-0.9700470, -0.0282520, +0.9704583),
    float3(-0.6815790, +0.2408420, +0.7228795),
    float3(-0.6794800, +0.4021560, +0.7895711),
    float3(-0.4395440, -0.6184920, +0.7587696),
    float3(-0.1461510, -0.6460370, +0.6623624),
    float3(-0.3946660, -0.0280020, +0.3956581),
    float3(-0.0554840, -0.2825210, +0.2879177),
    float3(-0.2190420, +0.0017410, +0.2190489),
    float3(-0.3102390, +0.1215030, +0.3331835),
    float3(-0.0824540, +0.6779430, +0.6829388),
    float3(-0.4505440, +0.5801740, +0.7345691),
    float3(+0.3955200, -0.8231480, +0.9132408),
    float3(+0.4313840, -0.6332400, +0.7662147),
    float3(+0.1415130, -0.2275940, +0.2680018),
    float3(+0.2887790, -0.1454870, +0.3233570),
    float3(+0.3987600, +0.3971440, +0.5627903),
    float3(+0.1581700, +0.0172060, +0.1591031),
    float3(+0.2391310, +0.6859850, +0.7264703),
    float3(+0.3127890, +0.8389260, +0.8953401),
    float3(+0.6418050, -0.2599680, +0.6924573),
    float3(+0.7577960, -0.4595200, +0.8862355),
    float3(+0.8806330, +0.2204820, +0.9078143),
    float3(+0.9603210, +0.0590630, +0.9621356),
    float3(-0.8280720, -0.2189210, +0.8565218),
    float3(-0.6851090, -0.1357820, +0.6984348),
    float3(-0.5356260, +0.0065290, +0.5356658),
    float3(-0.5174520, +0.1682760, +0.5441263),
    float3(-0.2839380, -0.5403500, +0.6104088),
    float3(-0.0438950, -0.5046060, +0.5065116),
    float3(-0.0533950, +0.1513330, +0.1604765),
    float3(-0.0230270, +0.2816990, +0.2826386),
    float3(-0.2871450, +0.5419330, +0.6133054),
    float3(+0.4847900, -0.1383370, +0.5041413),
    float3(+0.3547590, -0.4612630, +0.5819085),
    float3(+0.4383540, +0.0239030, +0.4390052),
    float3(+0.3910370, +0.6516810, +0.7599987),
    float3(+0.3202440, +0.5387190, +0.6267172),
    float3(+0.8158000, +0.0179200, +0.8159968),
    float3(-0.5388640, -0.1738260, +0.5662066),
    float3(+0.4655580, -0.2966590, +0.5520424)
};