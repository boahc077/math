#include <stan/math/rev.hpp>
#include <gtest/gtest.h>
#include <boost/math/differentiation/finite_difference.hpp>
#include <boost/math/special_functions/digamma.hpp>
#include <algorithm>
#include <limits>
#include <vector>

namespace neg_binomial_2_log_test_internal {
struct TestValue {
  int n;
  double eta;
  double phi;
  double value;
  double grad_eta;
  double grad_phi;

  TestValue(int _n, double _eta, double _phi, double _value, double _grad_eta,
            double _grad_phi)
      : n(_n),
        eta(_eta),
        phi(_phi),
        value(_value),
        grad_eta(_grad_eta),
        grad_phi(_grad_phi) {}
};

// Test data generated in Mathematica (Wolfram Cloud). The code can be re-ran at
// https://www.wolframcloud.com/obj/martin.modrak/Published/NegBinomial_2_Log_test.nb
// but is also presented below for convenience:
//
// nb2log[n_,eta_,phi_]:= LogGamma[n + phi] - LogGamma[n + 1] -
//     LogGamma[phi ] + n * (eta - Log[Exp[eta] + phi]) +
//     phi * (Log[phi] - Log[Exp[eta] + phi]);
// nb2logdeta[n_,eta_,phi_]= D[nb2log[n, eta, phi],eta];
// nb2logdphi[n_,eta_,phi_]= D[nb2log[n, eta, phi],phi];
// out = OpenWrite["nb2log_test.txt"]
// etas= {-130,-50,-1, -135*10^-3,0,98 *10^-2,5,368};
// phis=  {4*10^-4,65*10^-3,442*10^-2,800, 15324};
// ns = {0,6,14,1525,10233};
//  WriteString[out, "std::vector<TestValue> testValues = {"];
//    Block[{$MaxPrecision = 80, $MinPrecision = 40}, {
//      For[i = 1, i <= Length[etas], i++, {
//        For[j = 1, j <= Length[phis], j++, {
//        For[k = 1, k <= Length[ns], k++, {
//          ceta = etas[[i]];
//          cphi = phis[[j]];
//     cn=ns[[k]];
//          val = N[nb2log[cn,ceta,cphi]];
//     ddeta= N[nb2logdeta[cn,ceta,cphi]];
//     ddphi= N[nb2logdphi[cn,ceta,cphi]];
//          WriteString[out,"  TestValue(",CForm[cn],",",CForm[ceta],",",
//            CForm[cphi],",", CForm[val],","CForm[ddeta],",",
//            CForm[ddphi],"),"]
//        }]
//      }]
//   }]
//  }];
//  WriteString[out,"};"];
//  Close[out];
//  FilePrint[%]

std::vector<TestValue> testValues = {
    TestValue(0, -130, 0.0004, 0., -3.4811068399043105e-57, 0.),
    TestValue(6, -130, 0.0004, -742.670616198677, 6., -12497.717251921473),
    TestValue(14, -130, 0.0004, -1720.9251872606271, 14., -32496.820494410564),
    TestValue(1525, -130, 0.0004, -186333.4804666206, 1525.,
              -3.80999209402007e6),
    TestValue(10233, -130, 0.0004, -1.250243590665905e6, 10233.,
              -2.557999019011787e7),
    TestValue(0, -130, 0.065, 0., -3.4811068399043105e-57, 0.),
    TestValue(6, -130, 0.065, -767.9794906909146, 6., -74.73014818539053),
    TestValue(14, -130, 0.065, -1786.9017778439902, 14., -196.917187021929),
    TestValue(1525, -130, 0.065, -194091.16634050777, 1525.,
              -23438.34928792469),
    TestValue(10233, -130, 0.065, -1.3023307775882862e6, 10233.,
              -157405.6761910148),
    TestValue(0, -130, 4.42, 0., -3.4811068399043105e-57, 0.),
    TestValue(6, -130, 4.42, -784.0828117852828, 6., -0.4312637869430638),
    TestValue(14, -130, 4.42, -1833.6283513006856, 14., -1.6501470360217263),
    TestValue(1525, -130, 4.42, -200493.63376876694, 1525., -339.0590808438865),
    TestValue(10233, -130, 4.42, -1.3454684320637877e6, 10233.,
              -2307.29338827833),
    TestValue(0, -130, 800, 0., -3.4811068399043105e-57, 0.),
    TestValue(6, -130, 800, -786.5605440348701, 6., -0.000023330624470518056),
    TestValue(14, -130, 800, -1845.078105689095, 14., -0.00014060783924468162),
    TestValue(1525, -130, 800, -206952.5277963584, 1525., -0.8389763486176387),
    TestValue(10233, -130, 800, -1.395830983586415e6, 10233.,
              -10.166635855973585),
    TestValue(0, -130, 15324, 0., -3.4811068399043105e-57, 0.),
    TestValue(6, -130, 15324, -786.5782724723963, 6., -6.386208176026916e-8),
    TestValue(14, -130, 15324, -1845.1852845285289, 14., -3.872952154126844e-7),
    TestValue(1525, -130, 15324, -207834.0167357739, 1525.,
              -0.004643063079575183),
    TestValue(10233, -130, 15324, -1.4117087218942088e6, 10233.,
              -0.15627194802547817),
    TestValue(0, -50, 0.0004, 0., -1.9287498479639178e-22, 0.),
    TestValue(6, -50, 0.0004, -262.6706161986769, 6., -12497.717251921473),
    TestValue(14, -50, 0.0004, -600.9251872606274, 14., -32496.820494410564),
    TestValue(1525, -50, 0.0004, -64333.4804666206, 1525., -3.80999209402007e6),
    TestValue(10233, -50, 0.0004, -431603.59066590504, 10233.,
              -2.557999019011787e7),
    TestValue(0, -50, 0.065, 0., -1.9287498479639178e-22, 0.),
    TestValue(6, -50, 0.065, -287.9794906909146, 6., -74.73014818539053),
    TestValue(14, -50, 0.065, -666.9017778439904, 14., -196.917187021929),
    TestValue(1525, -50, 0.065, -72091.16634050779, 1525., -23438.34928792469),
    TestValue(10233, -50, 0.065, -483690.77758828626, 10233.,
              -157405.6761910148),
    TestValue(0, -50, 4.42, 0., -1.9287498479639178e-22, 0.),
    TestValue(6, -50, 4.42, -304.08281178528296, 6., -0.4312637869430638),
    TestValue(14, -50, 4.42, -713.6283513006858, 14., -1.6501470360217263),
    TestValue(1525, -50, 4.42, -78493.63376876694, 1525., -339.0590808438865),
    TestValue(10233, -50, 4.42, -526828.4320637881, 10233., -2307.29338827833),
    TestValue(0, -50, 800, 0., -1.9287498479639178e-22, 0.),
    TestValue(6, -50, 800, -306.5605440348702, 6., -0.000023330624470518056),
    TestValue(14, -50, 800, -725.0781056890954, 14., -0.00014060783924468162),
    TestValue(1525, -50, 800, -84952.52779635842, 1525., -0.8389763486176387),
    TestValue(10233, -50, 800, -577190.983586415, 10233., -10.166635855973585),
    TestValue(0, -50, 15324, 0., -1.9287498479639178e-22, 0.),
    TestValue(6, -50, 15324, -306.5782724723962, 6., -6.386208176026916e-8),
    TestValue(14, -50, 15324, -725.1852845285287, 14., -3.872952154126844e-7),
    TestValue(1525, -50, 15324, -85834.01673577388, 1525.,
              -0.004643063079575183),
    TestValue(10233, -50, 15324, -593068.7218942087, 10233.,
              -0.15627194802547817),
    TestValue(0, -1, 0.0004, -0.002730053093156532, -0.00039956554729340564,
              -5.8262188646578155),
    TestValue(6, -1, 0.0004, -9.624142649118, 0.00611722505162245,
              2480.164552716579),
    TestValue(14, -1, 0.0004, -10.479775574199106, 0.014806279183510258,
              2459.33867489777),
    TestValue(1525, -1, 0.0004, -16.81061433296054, 1.65595137834382,
              -1638.7975986621),
    TestValue(10233, -1, 0.0004, -28.176651635192684, 11.113986800903698,
              -25281.98225286028),
    TestValue(0, -1, 0.065, -0.12324467951140204, -0.05523977671804774,
              -1.0462292737439123),
    TestValue(6, -1, 0.065, -5.4791673253246715, 0.8457039108467774,
              2.6706427321759207),
    TestValue(14, -1, 0.065, -7.57003041826529, 2.046962160933211,
              -14.9203691826154),
    TestValue(1525, -1, 0.065, -257.79937372403765, 228.93461414600833,
              -3500.7778852403826),
    TestValue(10233, -1, 0.065, -1676.4055320453917, 1536.504219365091,
              -23615.329484008667),
    TestValue(0, -1, 4.42, -0.35336977067787206, -0.33961321498519975,
              -0.0031123429168939953),
    TestValue(6, -1, 4.42, -10.915869027469208, 5.199373371007742,
              -0.3300744524827952),
    TestValue(14, -1, 4.42, -29.10099183821663, 12.58468881899833,
              -1.409888798391908),
    TestValue(1525, -1, 4.42, -3890.907704212679, 1407.4861440582208,
              -312.5521835201079),
    TestValue(10233, -1, 4.42, -26229.892416216317, 9446.402009195977,
              -2129.4099898544955),
    TestValue(0, -1, 800, -0.36779488254126136, -0.3677103498239346,
              -1.0566589647709179e-7),
    TestValue(6, -1, 800, -12.931097379030476, 5.629531822552386,
              -0.000019989005837535087),
    TestValue(14, -1, 800, -39.45233698208112, 13.625854719054148,
              -0.00013266984123889644),
    TestValue(1525, -1, 800, -10228.596700235812, 1523.9313417958242,
              -0.8381002694655955),
    TestValue(10233, -1, 800, -75779.05593758884, 10227.928814637991,
              -10.160756617874249),
    TestValue(0, -1, 15324, -0.36787502543801764, -0.36787060979358055,
              -2.8815172470331163e-10),
    TestValue(6, -1, 15324, -12.946291536606935, 5.631985353162647,
              -5.475079230526231e-8),
    TestValue(14, -1, 15324, -39.55349564443637, 13.631793303770952,
              -3.656513370973221e-7),
    TestValue(1525, -1, 15324, -11109.421220654047, 1524.5955199749144,
              -0.004640674342975881),
    TestValue(10233, -1, 15324, -91652.33542736099, 10232.386474212051,
              -0.1562559175659044),
    TestValue(0, -0.135, 0.0004, -0.0030758014883219406, -0.0003998169579138641,
              -6.689961326020191),
    TestValue(6, -0.135, 0.0004, -9.620714324994289, 0.0023458143341247776,
              2488.7287085224075),
    TestValue(14, -0.135, 0.0004, -10.471315153383557, 0.006006656056842966,
              2480.473361726521),
    TestValue(1525, -0.135, 0.0004, -15.851716649474838, 0.6974481364352408,
              756.5961351210399),
    TestValue(10233, -0.135, 0.0004, -21.74031670264958, 4.68227435161399,
              -9203.56550062657),
    TestValue(0, -0.135, 0.065, -0.17355816530480334, -0.060499170784905056,
              -1.7393691464599734),
    TestValue(6, -0.135, 0.065, -4.983802576662806, 0.3549619875315516,
              9.446464847896287),
    TestValue(14, -0.135, 0.065, -6.347094690329737, 0.9089101986201605,
              1.814068917686857),
    TestValue(1525, -0.135, 0.065, -119.1564692857828, 105.53587856798117,
              -1603.1098530521738),
    TestValue(10233, -0.135, 0.065, -745.8016166677262, 708.5085063379321,
              -10877.707952603063),
    TestValue(0, -0.135, 4.42, -0.7972828882914406, -0.7295110644556051,
              -0.015332991818062247),
    TestValue(6, -0.135, 4.42, -6.77237913731839, 4.280202422708732,
              -0.22255005640011594),
    TestValue(14, -0.135, 4.42, -18.840964604379963, 10.959820405594515,
              -1.142704342330766),
    TestValue(1525, -0.135, 4.42, -2725.386686190179, 1272.5726669231467,
              -282.1292052356146),
    TestValue(10233, -0.135, 4.42, -18406.520499591774, 8543.336841294322,
              -1925.1970362834456),
    TestValue(0, -0.135, 800, -0.8732391466033107, -0.8727627283344418,
              -5.955228363774268e-7),
    TestValue(6, -0.135, 800, -8.250332475072994, 5.12069155120305,
              -0.00001574399672842758),
    TestValue(14, -0.135, 800, -27.85662652076425, 13.111963923919705,
              -0.00012211167739840934),
    TestValue(1525, -0.135, 800, -8910.940647628244, 1522.463533320778,
              -0.8368973142018654),
    TestValue(10233, -0.135, 800, -66934.48164579559, 10220.963511022857,
              -10.15268179368541),
    TestValue(0, -0.135, 15324, -0.8736910046546811, -0.873666098586244,
              -1.625299006491332e-9),
    TestValue(6, -0.135, 15324, -8.262305564366748, 5.125991823849705,
              -4.3164385630243066e-8),
    TestValue(14, -0.135, 15324, -27.949773736920378, 13.125535720430971,
              -3.3683352693505975e-7),
    TestValue(1525, -0.135, 15324, -9790.852373971315, 1524.0393891872175,
              -0.004637390943832059),
    TestValue(10233, -0.135, 15324, -82801.63401513056, 10231.542920615924,
              -0.15623387778406472),
    TestValue(0, 0, 0.0004, -0.0031297783723510477, -0.00039984006397441024,
              -6.824845770941593),
    TestValue(6, 0, 0.0004, -9.620421562314927, 0.001999200319872051,
              2489.460301347968),
    TestValue(14, 0, 0.0004, -10.470560071286346, 0.005197920831667333,
              2482.3602575793902),
    TestValue(1525, 0, 0.0004, -15.763640987330291, 0.6093562574970012,
              976.6908902568914),
    TestValue(10233, 0, 0.0004, -21.149006354171433, 4.091163534586165,
              -7725.923400264901),
    TestValue(0, 0, 0.065, -0.18176228253611273, -0.06103286384976526,
              -1.8573756720976538),
    TestValue(6, 0, 0.065, -4.939309822938063, 0.3051643192488263,
              10.086365633302695),
    TestValue(14, 0, 0.065, -6.2323394419969205, 0.7934272300469484,
              3.464512784485386),
    TestValue(1525, 0, 0.065, -105.77088536835072, 93.01408450704226,
              -1410.5930846874387),
    TestValue(10233, 0, 0.065, -655.9353073694074, 624.4882629107981,
              -9585.215040143006),
    TestValue(0, 0, 4.42, -0.9014860475798108, -0.8154981549815498,
              -0.019454274343498),
    TestValue(6, 0, 4.42, -6.20803454903441, 4.077490774907749,
              -0.2002630680488462),
    TestValue(14, 0, 4.42, -17.385223019332848, 10.601476014760147,
              -1.0852063261438887),
    TestValue(1525, 0, 4.42, -2555.5683368414957, 1242.819188191882,
              -275.42122433697745),
    TestValue(10233, 0, 4.42, -17266.416519266422, 8344.177121771218,
              -1880.16185158575),
    TestValue(0, 0, 800, -0.9993755203460353, -0.9987515605493134,
              -7.799497456772997e-7),
    TestValue(6, 0, 800, -7.567414871619192, 4.9937578027465666,
              -0.00001474727833628009),
    TestValue(14, 0, 800, -26.094970281047694, 12.983770287141073,
              -0.00011954009860382087),
    TestValue(1525, 0, 800, -8705.432231464434, 1522.0973782771537,
              -0.8365972908645132),
    TestValue(10233, 0, 800, -65554.76622405997, 10219.225967540575,
              -10.150667534799734),
    TestValue(0, 0, 15324, -0.9999673728476992, -0.9999347471451876,
              -2.129059595290528e-9),
    TestValue(6, 0, 15324, -7.578631375147566, 4.999673735725938,
              -4.0441863191631455e-8),
    TestValue(14, 0, 15324, -26.18616547115994, 12.99915171288744,
              -3.298092945414055e-7),
    TestValue(1525, 0, 15324, -9585.116216997136, 1523.9005546492658,
              -0.004636571433929149),
    TestValue(10233, 0, 15324, -81420.38961583155, 10231.33233278956,
              -0.15622837586171023),
    TestValue(0, 0.98, 0.0004, -0.003521678449611314, -0.0003999399592377696,
              -7.8043462259338625),
    TestValue(6, 0.98, 0.0004, -9.619314621296192, 0.0005006714742186634,
              2492.2268732689504),
    TestValue(14, 0.98, 0.0004, -10.46745467547295, 0.0017014867188272407,
              2490.1215926683385),
    TestValue(1525, 0.98, 0.0004, -15.383077442174908, 0.2285054660442723,
              1927.8381186954143),
    TestValue(10233, 0.98, 0.0004, -18.59312476501509, 1.5355928598007087,
              -1337.9764634954095),
    TestValue(0, 0.98, 0.065, -0.2429355772781735, -0.06345207263809681,
              -2.76128468677041),
    TestValue(6, 0.98, 0.065, -4.767248786178039, 0.0794335299991208,
              12.618019394958765),
    TestValue(14, 0.98, 0.065, -5.74929929656745, 0.26994766684874427,
              10.576916914579899),
    TestValue(1525, 0.98, 0.065, -46.55166607298088, 36.25330526432138,
              -538.291454718534),
    TestValue(10233, 0.98, 0.065, -258.21532828733325, 243.62794322513656,
              -3726.7666341288596),
    TestValue(0, 0.98, 4.42, -2.085194344338019, -1.6623571643545987,
              -0.09566452035824002),
    TestValue(6, 0.98, 4.42, -3.118586687545845, 2.0810493998699275,
              -0.01638680599463549),
    TestValue(14, 0.98, 4.42, -8.598233613515248, 7.072258152169295,
              -0.5545480533310743),
    TestValue(1525, 0.98, 4.42, -1470.6581882505307, 949.7868112427125,
              -209.3921137821333),
    TestValue(10233, 0.98, 4.42, -9979.732399673303, 6382.717538120574,
              -1436.6605223201655),
    TestValue(0, 0.98, 800, -2.6600289899732843, -2.655611540996233,
              -5.521811221598227e-6),
    TestValue(6, 0.98, 800, -3.3605232422682434, 3.324471372446295,
              -3.956077495042773e-6),
    TestValue(14, 0.98, 800, -14.064685186393206, 11.297915257036331,
              -0.0000880381480067513),
    TestValue(1525, 0.98, 800, -7215.758505610536, 1517.2821289589797,
              -0.8326540460538301),
    TestValue(10233, 0.98, 800, -55549.328711222974, 10196.375797335235,
              -10.124180638880098),
    TestValue(0, 0.98, 15324, -2.6642246279711728, -2.663993040879676,
              -1.5112705753494993e-8),
    TestValue(6, 0.98, 15324, -3.363540258013437, 3.33496389215052,
              -1.0907247577351953e-8),
    TestValue(14, 0.98, 15324, -14.13194319100731, 11.33357313619078,
              -2.435836623743224e-7),
    TestValue(1525, 0.98, 15324, -8092.446096303554, 1522.0708941042951,
              -0.004625777692748342),
    TestValue(10233, 0.98, 15324, -71394.82522420201, 10228.557056242118,
              -0.15615587395017738),
    TestValue(0, 5, 0.0004, -0.005129619482412584, -0.0003999989219313857,
              -11.824051401202995),
    TestValue(6, 5, 0.0004, -9.620038054348033, -0.0003838278927170233,
              2490.418269104288),
    TestValue(14, 5, 0.0004, -10.466998764550176, -0.00036226652043120673,
              2491.261123184485),
    TestValue(1525, 5, 0.0004, -15.159872938045737, 0.003710137670052396,
              2485.8065870493465),
    TestValue(10233, 5, 0.0004, -17.08620434453769, 0.027179691403163724,
              2429.0366049190375),
    TestValue(0, 5, 0.065, -0.5026973821845353, -0.06497154463642642,
              -6.734243654586291),
    TestValue(6, 5, 0.065, -4.885023351671648, -0.062344895691172866,
              10.80289048394233),
    TestValue(14, 5, 0.065, -5.677757542844056, -0.058842697097501465,
              11.638894745962721),
    TestValue(1525, 5, 0.065, -10.72300452714444, 0.6026350622821846,
              6.184059083508757),
    TestValue(10233, 5, 0.065, -16.31585327378707, 4.414778231493505,
              -50.560431225223354),
    TestValue(0, 5, 4.42, -15.66097567784475, -4.292171719051572,
              -2.5721275924871447),
    TestValue(6, 5, 4.42, -11.003030464726871, -4.1186491657279145,
              -1.685183812308864),
    TestValue(14, 5, 4.42, -8.894227315595337, -3.887285761296371,
              -1.146456971892401),
    TestValue(1525, 5, 4.42, -37.685674017920974, 39.81147725071138,
              -6.586785126365316),
    TestValue(10233, 5, 4.42, -286.73197869332216, 291.6505429744463,
              -61.66251014536436),
    TestValue(0, 5, 800, -136.14280118600206, -125.18861230732865,
              -0.013692736098342095),
    TestValue(6, 5, 800, -113.72441622976723, -120.12752689963362,
              -0.012542423482431009),
    TestValue(14, 5, 800, -93.60340589585249, -113.3794130227069,
              -0.011094843043363584),
    TestValue(1525, 5, 800, -1473.1928123052458, 1161.170595481826,
              -0.5543680944524236),
    TestValue(10233, 5, 800, -16253.562993271455, 8506.492550516554,
              -8.17868004560178),
    TestValue(0, 5, 15324, -147.69907423135155, -146.9895630824659,
              -0.000046300649236385993),
    TestValue(6, 5, 15324, -124.33517719483288, -141.0471157696556,
              -0.00004260878910443758),
    TestValue(14, 5, 15324, -103.0192965724122, -133.1238526859085,
              -0.000037924592620959194),
    TestValue(1525, 5, 15324, -2121.414393156032, 1363.3824622568213,
              -0.0037347843330450686),
    TestValue(10233, 5, 15324, -30500.05087098574, 9987.854328915519,
              -0.14991286444070076),
    TestValue(0, 368, 0.0004, -0.15032961840434253, -0.0004000000000000001,
              -374.8240460108563),
    TestValue(6, 368, 0.0004, -9.765221882218956, -0.0004000000000000001,
              2127.4587020676695),
    TestValue(14, 368, 0.0004, -10.61216103101976, -0.0004000000000000001,
              2128.35545957858),
    TestValue(1525, 368, 0.0004, -15.300962794837687, -0.0004000000000000001,
              2133.081933919416),
    TestValue(10233, 368, 0.0004, -17.20382461596455, -0.0004000000000000001,
              2134.9858361205343),
    TestValue(0, 368, 0.065, -24.097668920590625, -0.065, -369.7333680090865),
    TestValue(6, 368, 0.065, -28.477367666024247, -0.065, -352.1558238867848),
    TestValue(14, 368, 0.065, -29.266598891792, -0.065, -351.2659396464002),
    TestValue(1525, 368, 0.065, -33.65022328528903, -0.065, -346.5441943953198),
    TestValue(10233, 368, 0.065, -35.430094188966905, -0.065,
              -344.6403282546427),
    TestValue(0, 368, 4.42, -1619.991262543284, -4.420000000000001,
              -365.5138603039104),
    TestValue(6, 368, 4.42, -1615.1572361520293, -4.420000000000001,
              -364.58765802750503),
    TestValue(14, 368, 4.42, -1612.8136580987152, -4.420000000000001,
              -363.99658652545247),
    TestValue(1525, 368, 4.42, -1597.2619947735784, -4.420000000000001,
              -359.55031671340777),
    TestValue(10233, 368, 4.42, -1590.7558162463538, -4.420000000000001,
              -357.6488775415163),
    TestValue(0, 368, 800, -289052.31061786565, -800., -360.3153882723321),
    TestValue(6, 368, 800, -289018.7634915345, -800., -360.30791160295655),
    TestValue(14, 368, 800, -288983.8041593674, -800., -360.29802888017133),
    TestValue(1525, 368, 800, -287560.8055295305, -800., -359.2481146209497),
    TestValue(10233, 368, 800, -286189.66239505477, -800., -357.6907741283057),
    TestValue(0, 368, 15324, -5.491551922548806e6, -15324.,
              -357.36282449417945),
    TestValue(6, 368, 15324, -5.4915006777682435e6, -15324.,
              -357.36243301536337),
    TestValue(14, 368, 15324, -5.491442187376253e6, -15324.,
              -357.36191128189233),
    TestValue(1525, 368, 15324, -5.486439246638204e6, -15324.,
              -357.2679504598954),
    TestValue(10233, 368, 15324, -5.474353427491954e6, -15324.,
              -356.85132040461684),
};

}  // namespace neg_binomial_2_log_test_internal

TEST(ProbDistributionsNegBinomial2Log, derivativesPrecomputed) {
  using neg_binomial_2_log_test_internal::TestValue;
  using neg_binomial_2_log_test_internal::testValues;
  using stan::math::is_nan;
  using stan::math::neg_binomial_2_log_lpmf;
  using stan::math::value_of;
  using stan::math::var;

  for (TestValue t : testValues) {
    int n = t.n;
    var eta(t.eta);
    var phi(t.phi);
    var val = neg_binomial_2_log_lpmf(n, eta, phi);

    std::vector<var> x;
    x.push_back(eta);
    x.push_back(phi);

    std::vector<double> gradients;
    val.grad(x, gradients);

    for (int i = 0; i < 2; ++i) {
      EXPECT_FALSE(is_nan(gradients[i]));
    }

    auto tolerance = [](double x) { return std::max(fabs(x * 1e-8), 1e-14); };

    EXPECT_NEAR(value_of(val), t.value, tolerance(t.value))
        << "value n = " << n << ", eta = " << t.eta << ", phi = " << t.phi;
    EXPECT_NEAR(gradients[0], t.grad_eta, tolerance(t.grad_eta))
        << "grad_mu n = " << n << ", eta = " << t.eta << ", phi = " << t.phi;
    EXPECT_NEAR(gradients[1], t.grad_phi, tolerance(t.grad_phi))
        << "grad_phi n = " << n << ", eta = " << t.eta << ", phi = " << t.phi;
  }
}

TEST(ProbDistributionsNegBinomial2Log, derivativesComplexStep) {
  using boost::math::differentiation::complex_step_derivative;
  using stan::math::is_nan;
  using stan::math::neg_binomial_2_log_lpmf;
  using stan::math::var;

  // TODO(martinmodrak) Reduced span of test, as the quick fix is not stable
  // enough should be resolved once PR #1497 is merged
  // std::vector<int> n_to_test = {0, 7, 100, 835, 14238, 385000, 1000000};
  // std::vector<double> eta_to_test =
  //   {-1531,-831, -124.5, -13, -2, 0, 0.536844, 1.26845, 11, 850, 2423};
  std::vector<int> n_to_test = {0, 7, 100, 835};
  std::vector<double> eta_to_test = {-2, 0, 0.536844, 1.26845, 11};

  auto nb2_log_for_test = [](int n, const std::complex<double>& eta,
                             const std::complex<double>& phi) {
    // Using first-order Taylor expansion of lgamma(a + b*i) around b = 0
    // Which happens to work nice in this case, as b is always 0 or the very
    // small complex step
    auto lgamma_c_approx = [](const std::complex<double>& x) {
      return std::complex<double>(lgamma(x.real()),
                                  x.imag() * boost::math::digamma(x.real()));
    };

    const double n_(n);
    return lgamma_c_approx(n_ + phi) - lgamma(n + 1) - lgamma_c_approx(phi)
           + phi * (log(phi) - log(exp(eta) + phi)) - n_ * log(exp(eta) + phi)
           + n_ * eta;
  };

  double phi_cutoff = 1e10;
  for (double eta_dbl : eta_to_test) {
    for (int n : n_to_test) {
      for (double phi_dbl = 1.5; phi_dbl < 1e22; phi_dbl *= 10) {
        var eta(eta_dbl);
        var phi(phi_dbl);
        var val = neg_binomial_2_log_lpmf(n, eta, phi);

        std::vector<var> x;
        x.push_back(eta);
        x.push_back(phi);

        std::vector<double> gradients;
        val.grad(x, gradients);

        EXPECT_TRUE(value_of(val) < 0)
            << "for n = " << n << ", eta = " << eta_dbl
            << ", phi = " << phi_dbl;

        for (int i = 0; i < 2; ++i) {
          EXPECT_FALSE(is_nan(gradients[i]));
        }

        auto nb2_log_eta
            = [n, phi_dbl, nb2_log_for_test](const std::complex<double>& eta) {
                return nb2_log_for_test(n, eta, phi_dbl);
              };
        auto nb2_log_phi
            = [n, eta_dbl, nb2_log_for_test](const std::complex<double>& phi) {
                return nb2_log_for_test(n, eta_dbl, phi);
              };
        double complex_step_deta
            = complex_step_derivative(nb2_log_eta, eta_dbl);
        double complex_step_dphi
            = complex_step_derivative(nb2_log_phi, phi_dbl);

        std::ostringstream message;
        message << ", n = " << n << ", eta = " << eta_dbl
                << ", phi = " << phi_dbl;

        double tolerance_phi;
        double tolerance_eta;
        if (phi < phi_cutoff || n < 100000) {
          tolerance_phi = std::max(1e-10, fabs(gradients[1]) * 1e-8);
        } else {
          tolerance_phi = std::max(1e-8, fabs(gradients[1]) * 1e-5);
        }

        if (phi < phi_cutoff) {
          tolerance_eta = std::max(1e-10, fabs(gradients[0]) * 1e-8);
        } else {
          tolerance_eta = std::max(1e-8, fabs(gradients[0]) * 1e-5);
        }

        EXPECT_NEAR(gradients[0], complex_step_deta, tolerance_eta)
            << "grad_mu" << message.str();

        EXPECT_NEAR(gradients[1], complex_step_dphi, tolerance_phi)
            << "grad_phi" << message.str();
      }
    }
  }
}

TEST(ProbDistributionsNegBinomial2Log, derivatives_diff_sizes) {
  using stan::math::neg_binomial_2_log_lpmf;
  using stan::math::var;

  int N = 100;
  double mu_dbl = 1.5;
  std::vector<double> phi_dbl{2, 4, 6, 8};

  var mu(mu_dbl);
  std::vector<var> phi;
  for (double i : phi_dbl) {
    phi.push_back(var(i));
  }
  var val = neg_binomial_2_log_lpmf(N, mu, phi);

  std::vector<var> x{mu};
  std::vector<double> gradients;
  val.grad(x, gradients);

  double eps = 1e-6;
  double grad_diff = (neg_binomial_2_log_lpmf(N, mu_dbl + eps, phi_dbl)
                      - neg_binomial_2_log_lpmf(N, mu_dbl - eps, phi_dbl))
                      / (2 * eps);
  EXPECT_FLOAT_EQ(grad_diff, gradients[0]);
}
