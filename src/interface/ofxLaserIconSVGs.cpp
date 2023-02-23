//
//  ofxLaserIconSVGs.cpp
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 11/02/2023.
//
//

#include "ofxLaserIconSVGs.h"

using namespace ofxLaser;

IconSVGs :: IconSVGs () {
    initSVGs();
}

void IconSVGs :: initSVGs() {
    
    iconGrabOpen.loadFromString("<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" version=\"1.1\" x=\"0px\" y=\"0px\" viewBox=\"0 0 24 30\" style=\"enable-background:new 0 0 24 24;\" xml:space=\"preserve\"><g><g><path fill=\"white\"  d=\"M16.313,23.134H3.384c-0.827,0-1.5-0.673-1.5-1.5c0-1.388,1.036-2.582,2.409-2.778l4.03-0.576l-7.543-7.543 c-0.428-0.428-0.665-0.998-0.665-1.604S0.352,7.958,0.781,7.53c0.325-0.325,0.731-0.539,1.173-0.624    C1.147,6.017,1.173,4.638,2.031,3.78c0.858-0.858,2.239-0.883,3.127-0.076C5.24,3.274,5.448,2.863,5.781,2.53    c0.884-0.885,2.323-0.885,3.207,0l0.171,0.171c0.083-0.429,0.29-0.839,0.622-1.171c0.884-0.885,2.323-0.885,3.207,0l8.982,8.982    c1.234,1.234,1.914,2.875,1.914,4.621s-0.68,3.387-1.914,4.621l-1.768,1.768C19.163,22.561,17.782,23.134,16.313,23.134z     M2.384,7.866c-0.325,0-0.649,0.124-0.896,0.371c-0.494,0.494-0.494,1.299,0,1.793l8.25,8.25c0.134,0.134,0.181,0.332,0.121,0.512    c-0.06,0.18-0.216,0.31-0.403,0.337l-5.02,0.717c-0.884,0.126-1.551,0.895-1.551,1.788c0,0.276,0.224,0.5,0.5,0.5h12.929    c1.202,0,2.332-0.468,3.182-1.318l1.768-1.768c1.045-1.045,1.621-2.436,1.621-3.914s-0.576-2.869-1.621-3.914l-8.982-8.982    c-0.494-0.494-1.299-0.494-1.793,0s-0.494,1.299,0,1.793l4.75,4.75c0.098,0.098,0.146,0.226,0.146,0.354s-0.049,0.256-0.146,0.354    c-0.195,0.195-0.512,0.195-0.707,0l-6.25-6.25c-0.479-0.479-1.313-0.479-1.793,0c-0.494,0.494-0.494,1.299,0,1.793l6.25,6.25    c0.098,0.098,0.146,0.226,0.146,0.354s-0.049,0.256-0.146,0.354c-0.195,0.195-0.512,0.195-0.707,0l-7.5-7.5    c-0.494-0.494-1.299-0.494-1.793,0s-0.494,1.299,0,1.793l7.5,7.5c0.098,0.098,0.146,0.226,0.146,0.354s-0.049,0.256-0.146,0.354    c-0.195,0.195-0.512,0.195-0.707,0l-6.25-6.25C3.033,7.99,2.709,7.866,2.384,7.866z\"/></g></g></svg>");
    
    iconGrabClosed.loadFromString("<svg xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' version='1.1' x='0px' y='0px' viewBox='0 0 24 30' style='enable-background:new 0 0 24 24;' xml:space='preserve'><g><g><path fill='white' d='M14.058,21.129H9.129c-2.481,0-4.5-2.019-4.5-4.5v-1.793l-1.595-1.595c-0.884-0.884-0.884-2.323,0-3.207    c0.336-0.337,0.76-0.555,1.221-0.632C3.663,8.521,3.756,7.313,4.534,6.534C5.238,5.83,6.293,5.688,7.139,6.103    C7.199,5.62,7.415,5.154,7.784,4.784c0.857-0.858,2.237-0.883,3.126-0.077c0.084-0.441,0.298-0.848,0.624-1.173    c0.884-0.885,2.323-0.885,3.207,0l4.974,4.974c1.234,1.234,1.914,2.875,1.914,4.621s-0.68,3.387-1.914,4.621l-1.768,1.768    C16.908,20.557,15.527,21.129,14.058,21.129z M5.629,15.836v0.793c0,1.93,1.57,3.5,3.5,3.5h4.929c1.202,0,2.332-0.468,3.182-1.318    l1.768-1.768c1.045-1.045,1.621-2.436,1.621-3.914s-0.576-2.869-1.621-3.914l-4.974-4.974c-0.479-0.479-1.313-0.479-1.793,0    c-0.494,0.494-0.494,1.299,0,1.793l0.241,0.241c0.098,0.098,0.146,0.226,0.146,0.354s-0.049,0.256-0.146,0.354    c-0.195,0.195-0.512,0.195-0.707,0l-1.491-1.491c-0.494-0.494-1.299-0.494-1.793,0C8.252,5.731,8.12,6.05,8.12,6.388    s0.132,0.657,0.371,0.896l1.491,1.491c0.098,0.098,0.146,0.226,0.146,0.354S10.08,9.385,9.982,9.483    c-0.195,0.195-0.512,0.195-0.707,0L7.034,7.241c-0.494-0.494-1.299-0.494-1.793,0s-0.494,1.299,0,1.793l2.241,2.241    c0.098,0.098,0.146,0.226,0.146,0.354s-0.049,0.256-0.146,0.354c-0.195,0.195-0.512,0.195-0.707,0l-1.241-1.241    c-0.479-0.479-1.313-0.479-1.793,0C3.502,10.981,3.37,11.3,3.37,11.638s0.132,0.657,0.371,0.896l3.741,3.741    c0.098,0.098,0.146,0.226,0.146,0.354s-0.049,0.256-0.146,0.354c-0.195,0.195-0.512,0.195-0.707,0L5.629,15.836z'/></g></g></svg>");
    
    iconMagPlus.loadFromString("<?xml version='1.0' encoding='UTF-8' standalone='no'?> <!-- Created with Vectornator for iOS (http://vectornator.io/) --><!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN' 'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'><svg height='100%' style='fill-rule:nonzero;clip-rule:evenodd;stroke-linecap:round;stroke-linejoin:round;' xmlns:xlink='http://www.w3.org/1999/xlink' xmlns='http://www.w3.org/2000/svg' xml:space='preserve' width='100%' xmlns:vectornator='http://vectornator.io' version='1.1' viewBox='0 0 25 25'><defs/><g id='Untitled' vectornator:layerName='Untitled'><g opacity='1'><path d='M23.359+20.1039L20.4339+17.1788C19.5811+18.4307+18.4979+19.5139+17.246+20.3666L20.1711+23.2918C21.0517+24.1724+22.4798+24.1724+23.359+23.2918C24.2396+22.4111+24.2396+20.9845+23.359+20.1039Z' opacity='1' fill='#ffffff'/><path d='M21.0134+10.4223C21.0134+4.61158+16.3035-0.098298+10.4928-0.098298C4.68212-0.098298-0.0277553+4.61158-0.0277553+10.4223C-0.0277553+16.233+4.68212+20.9428+10.4928+20.9428C16.3035+20.9428+21.0134+16.2329+21.0134+10.4223ZM10.4928+18.6884C5.93482+18.6884+2.22665+14.9795+2.22665+10.4223C2.22665+5.86428+5.93482+2.15611+10.4928+2.15611C15.05+2.15611+18.759+5.86428+18.759+10.4223C18.759+14.9795+15.05+18.6884+10.4928+18.6884Z' opacity='1' fill='#ffffff'/></g><path d='M14.0491+8.87293L6.96295+8.88761C6.0614+8.88952+5.33189+9.62109+5.33379+10.5226C5.3357+11.4242+6.06874+12.1537+6.97029+12.1518L14.0564+12.1371C14.958+12.1352+15.6875+11.4036+15.6856+10.5021C15.6837+9.60054+14.9506+8.87103+14.0491+8.87293ZM12.1612+14.0485L12.1466+6.96241C12.1447+6.06085+11.4131+5.33134+10.5115+5.33325C9.60997+5.33515+8.88046+6.06819+8.88237+6.96975L8.89704+14.0559C8.89895+14.9574+9.63052+15.6869+10.5321+15.685C11.4336+15.6831+12.1631+14.9501+12.1612+14.0485Z' opacity='1' fill='#ffffff'/></g></svg>");
    
    iconMagMinus.loadFromString("<?xml version='1.0' encoding='UTF-8' standalone='no'?> <!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN' 'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'><svg height='100%' style='fill-rule:nonzero;clip-rule:evenodd;stroke-linecap:round;stroke-linejoin:round;' xmlns:xlink='http://www.w3.org/1999/xlink' xmlns='http://www.w3.org/2000/svg' xml:space='preserve' width='100%' version='1.1' viewBox='0 0 25 25'><defs/><g id='Untitled' vectornator:layerName='Untitled'><g opacity='1'><path d='M23.359+20.1039L20.4339+17.1788C19.5811+18.4307+18.4979+19.5139+17.246+20.3666L20.1711+23.2918C21.0517+24.1724+22.4798+24.1724+23.359+23.2918C24.2396+22.4111+24.2396+20.9845+23.359+20.1039Z' fill-rule='evenodd' fill='#ffffff' opacity='1'/><path d='M21.0134+10.4223C21.0134+4.61158+16.3035-0.098298+10.4928-0.098298C4.68212-0.098298-0.0277553+4.61158-0.0277553+10.4223C-0.0277553+16.233+4.68212+20.9428+10.4928+20.9428C16.3035+20.9428+21.0134+16.2329+21.0134+10.4223ZM10.4928+18.6884C5.93482+18.6884+2.22665+14.9795+2.22665+10.4223C2.22665+5.86428+5.93482+2.15611+10.4928+2.15611C15.05+2.15611+18.759+5.86428+18.759+10.4223C18.759+14.9795+15.05+18.6884+10.4928+18.6884Z' fill-rule='evenodd' fill='#ffffff' opacity='1'/></g><path d='M14.0491+8.87293L6.96295+8.88761C6.0614+8.88952+5.33189+9.62109+5.33379+10.5226C5.3357+11.4242+6.06874+12.1537+6.97029+12.1518L14.0564+12.1371C14.958+12.1352+15.6875+11.4036+15.6856+10.5021C15.6837+9.60054+14.9506+8.87103+14.0491+8.87293Z' opacity='1' fill='#ffffff'/></g></svg>");
    numberSVGs.resize(10);
    numberSVGs[0].loadFromString("<svg>\
         <path unicode='&#x30;' d='M1100 595Q1100 300 967.50 140Q835-20 594-20Q351-20 218 141Q85 302 85 595L85 861Q85 1156 217.50 1316Q350 1476 592 1476Q834 1476 967 1315Q1100 1154 1100 860L1100 595M763 904Q763 1061 722 1138.50Q681 1216 592 1216Q505 1216 465.50 1144Q426 1072 423 926L423 554Q423 391 464 315.50Q505 240 594 240Q680 240 721 313.50Q762 387 763 544L763 904Z' fill='#ffffff'/>                  \
        </svg>");
    numberSVGs[1].loadFromString("<svg>\
         <path unicode='&#x31;' d='M836 0L498 0L498 1076L166 979L166 1235L805 1456L836 1456L836 0Z' fill='#ffffff'/>                  \
        </svg>");
    numberSVGs[2].loadFromString("<svg>\
         <path unicode='&#x32;' d='M1105 0L89 0L89 220L557 712Q730 909 730 1025Q730 1119 689 1168Q648 1217 570 1217Q493 1217 445 1151.50Q397 1086 397 988L59 988Q59 1122 126 1235.50Q193 1349 312 1413Q431 1477 578 1477Q814 1477 941.50 1368Q1069 1259 1069 1055Q1069 969 1037 887.50Q1005 806 937.50 716.50Q870 627 720 477L532 260L1105 260L1105 0Z' fill='#ffffff'/>                  \
        </svg>");
    numberSVGs[3].loadFromString("<svg>\
         <path unicode='&#x33;' d='M397 869L556 869Q745 869 745 1054Q745 1126 700 1171.50Q655 1217 573 1217Q506 1217 456.50 1178Q407 1139 407 1081L70 1081Q70 1196 134 1286Q198 1376 311.50 1426.50Q425 1477 561 1477Q804 1477 943 1366Q1082 1255 1082 1061Q1082 967 1024.50 883.50Q967 800 857 748Q973 706 1039 620.50Q1105 535 1105 409Q1105 214 955 97Q805-20 561-20Q418-20 295.50 34.50Q173 89 110 185.50Q47 282 47 405L386 405Q386 338 440 289Q494 240 573 240Q662 240 715 289.50Q768 339 768 416Q768 526 713 572Q658 618 561 618L397 618L397 869Z' fill='#ffffff'/>                  \
        </svg>");
    numberSVGs[4].loadFromString("<svg>\
         <path unicode='&#x34;' d='M979 569L1127 569L1127 309L979 309L979 0L642 0L642 309L79 309L59 515L642 1453L642 1456L979 1456L979 569M380 569L642 569L642 1017L621 983L380 569Z' fill='#ffffff'/>                  \
        </svg>");
    numberSVGs[5].loadFromString("<svg>\
         <path unicode='&#x35;' d='M109 712L198 1456L1049 1456L1049 1194L472 1194L439 905Q475 926 533.50 942Q592 958 648 958Q865 958 981.50 829.50Q1098 701 1098 469Q1098 329 1035.50 215.50Q973 102 860 41Q747-20 593-20Q456-20 336 36.50Q216 93 148 191.50Q80 290 81 414L419 414Q424 334 470 287Q516 240 591 240Q761 240 761 492Q761 725 553 725Q435 725 377 649L109 712Z' fill='#ffffff'/>                  \
        </svg>");
    numberSVGs[6].loadFromString("<svg>\
         <path unicode='&#x36;' d='M903 1477L903 1212L888 1212Q699 1212 577.50 1121.50Q456 1031 428 870Q542 982 716 982Q905 982 1017 844Q1129 706 1129 482Q1129 343 1063.50 227Q998 111 880.50 45.50Q763-20 620-20Q465-20 343 50.50Q221 121 153 252Q85 383 83 554L83 689Q83 914 179.50 1093.50Q276 1273 455 1375Q634 1477 852 1477L903 1477M599 724Q532 724 487 693Q442 662 420 617L420 516Q420 240 611 240Q688 240 739.50 309Q791 378 791 482Q791 589 738.50 656.50Q686 724 599 724Z' fill='#ffffff'/>                  \
        </svg>");
    numberSVGs[7].loadFromString("<svg>\
         <path unicode='&#x37;' d='M1101 1276L557 0L201 0L746 1196L52 1196L52 1456L1101 1456L1101 1276Z' fill='#ffffff'/>                  \
        </svg>");
    numberSVGs[8].loadFromString("<svg>\
         <path unicode='&#x38;' d='M1071 1067Q1071 961 1018.50 880Q966 799 874 750Q978 699 1039 610.50Q1100 522 1100 401Q1100 206 965.50 93Q831-20 595-20Q358-20 221 93.50Q84 207 84 401Q84 518 144 608Q204 698 314 750Q221 799 168 880Q115 961 115 1067Q115 1257 244 1367Q373 1477 592 1477Q813 1477 942 1367Q1071 1257 1071 1067M762 428Q762 518 715.50 566.50Q669 615 593 615Q517 615 470 566.50Q423 518 423 428Q423 341 470.50 290.50Q518 240 595 240Q670 240 716 290Q762 340 762 428M592 1217Q524 1217 488.50 1172.50Q453 1128 453 1049Q453 971 489 923Q525 875 595 875Q664 875 698.50 923Q733 971 733 1049Q733 1127 697.50 1172Q662 1217 592 1217Z' fill='#ffffff'/>                  \
        </svg>");
    numberSVGs[9].loadFromString("<svg>\
         <path unicode='&#x39;' d='M753 563Q644 465 509 465Q311 465 195 599Q79 733 79 957Q79 1097 144 1217Q209 1337 326.50 1407Q444 1477 586 1477Q730 1477 846.50 1404.50Q963 1332 1028 1199.50Q1093 1067 1095 894L1095 765Q1095 524 1000.50 347Q906 170 731 75Q556-20 323-20L302-20L302 250L366 251Q714 268 753 563M600 708Q710 708 758 803L758 943Q758 1083 711 1150Q664 1217 584 1217Q513 1217 465 1141Q417 1065 417 957Q417 843 466 775.50Q515 708 600 708Z' fill='#ffffff'/>                  \
                     </svg>");
    
}