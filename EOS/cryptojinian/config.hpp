#ifndef _CONFIG_HPP

#include <eosiolib/asset.hpp>
#include "utils.hpp"

namespace config {

// basic
const auto EOS_SYMBOL = eosio::symbol("EOS", 4);
const auto EOS_CONTRACT = "eosio.token"_n;

constexpr auto CCC_MAX_SUPPLY = 300000000000;

const auto CCC_SYMBOL = eosio::symbol("CCC", 4);
// const auto CCC_CONTRACT = get_self();

const auto TOKEN_SYMBOL = CCC_SYMBOL ;
// const auto TOKEN_CONTRACT =

const auto DIVIDEND_EX_SYMBOL = eosio::extended_symbol( EOS_SYMBOL, EOS_CONTRACT ) ;

const auto DEF_SPONSOR = "rukamoemoe51"_n;

constexpr double_t TRADE_FEE = 0.0175 ; // 1.75%
constexpr double_t TRADE_COEF = 1 - TRADE_FEE ;

constexpr int64_t MINING_COEF = 10 ;

constexpr uint64_t PRICE_SCALE = 100000000;

constexpr uint32_t MINING_TIMES = 16 ;

constexpr double_t DIVIDEND_COEF{0.60} ;    // 分红比例降低到60%
constexpr double_t BUYBACK_COEF{0.25} ;      // 回购比例调整到25%
constexpr double_t COLLECTION_COEF{0.15} ;  // 集齐整版纪念币奖励调成15%

constexpr uint32_t FROZEN_DAYS = 30 ;

const asset cost_table( const uint64_t &n ) {
    if ( n > 408120 ) return asset( string_to_price("1.0000"), EOS_SYMBOL ) * 2 ;
    else if ( n > 386640 ) return asset( string_to_price("1.1000"), EOS_SYMBOL ) * 2 ;
    else if ( n > 365160 ) return asset( string_to_price("1.2100"), EOS_SYMBOL ) * 2 ;
    else if ( n > 343680 ) return asset( string_to_price("1.3310"), EOS_SYMBOL ) * 2 ;
    else if ( n > 322200 ) return asset( string_to_price("1.4640"), EOS_SYMBOL ) * 2 ;
    else if ( n > 300720 ) return asset( string_to_price("1.6110"), EOS_SYMBOL ) * 2 ;
    else if ( n > 279240 ) return asset( string_to_price("1.7720"), EOS_SYMBOL ) * 2 ;
    else if ( n > 257760 ) return asset( string_to_price("1.9490"), EOS_SYMBOL ) * 2 ;
    else if ( n > 236280 ) return asset( string_to_price("2.1440"), EOS_SYMBOL ) * 2 ;
    else if ( n > 214800 ) return asset( string_to_price("2.3580"), EOS_SYMBOL ) * 2 ;
    else if ( n > 193320 ) return asset( string_to_price("2.5940"), EOS_SYMBOL ) * 2 ;
    else if ( n > 171840 ) return asset( string_to_price("2.8530"), EOS_SYMBOL ) * 2 ;
    else if ( n > 150360 ) return asset( string_to_price("3.1380"), EOS_SYMBOL ) * 2 ;
    else if ( n > 128880 ) return asset( string_to_price("3.4520"), EOS_SYMBOL ) * 2 ;
    else if ( n > 107400 ) return asset( string_to_price("3.7970"), EOS_SYMBOL ) * 2 ;
    else if ( n > 85920 ) return asset( string_to_price("4.1770"), EOS_SYMBOL ) * 2 ;
    else if ( n > 64440 ) return asset( string_to_price("4.5950"), EOS_SYMBOL ) * 2 ;
    else if ( n > 42960 ) return asset( string_to_price("5.0540"), EOS_SYMBOL ) * 2 ;
    else if ( n > 21480 ) return asset( string_to_price("5.5600"), EOS_SYMBOL ) * 2 ;
    else return asset( string_to_price("6.1160"), EOS_SYMBOL ) * 2 ;

}

const vector<vector<uint64_t>> _coinvalues = { // [][+1]
        {1,1,2,5,10},   //btc 0
        {1,1,2,5,10},   //eth 1
        {1,1,2,5,10},   //lt 2
        {1,1,5,10,50,100},   //ba 3
        {1,1,5,10,20,50},   //ri 4
        {1,1,2,5,10},   //og 5
        {1,1,2,5,10,20},   //ae 6
        {1,1,2,5,10},   //as 7
        {1,1,2,5,10,20,50,100},   //ud 8
        {1,1,2,5,10},   //pt 9
        {1,1,2,5,10},   //mo 10
        {1,1,2,5,10},   //qt 11
        {5,5,10,20,50,100},   //bt 12
        {5,5,10,20,50},   //ht 13
        {5,5,10,20,50,100},   //eos 14
        {10,10,20,50,100},   //io 15
        {10,10,20,50,100},   //zb 16
        {50,50,100,200,500,1000},   //xlma 17
        {100,100,200,500,1000},   //ada 18
        {500,500,1000,2000,5000},   //dg 19
        {500,500,1000,2000,5000},   //rp 20
        {500,500,1000,2000,5000}    //tr 21
};

const vector<uint32_t> collection_combination_parameters( const uint32_t &type ) {
    vector<uint32_t> v ;
    switch( type ) {
        case 22 :
            v.push_back(0);
            v.push_back(2);
            v.push_back(10);
            return v;
        case 23 :
            v.push_back(1);
            v.push_back(14);
            v.push_back(7);
            v.push_back(11);
            v.push_back(12);
            v.push_back(6);
            v.push_back(15);
            v.push_back(18);
            v.push_back(21);
            return v;
        case 24 :
            v.push_back(3);
            v.push_back(13);
            v.push_back(16);
            return v;
        case 25 :
            v.push_back(17);
            v.push_back(5);
            v.push_back(20);
            return v;
        case 26 :
            v.push_back(9);
            v.push_back(8);
            return v;
        case 27 :
            v.push_back(19);
            v.push_back(4);
            return v;
        default :
            v.push_back(type);
            return v;
    }
}

const asset bouns_table( const uint32_t &type ) {
    switch( type ) {
        case 0 :
            return asset( string_to_price("10000.0000"), TOKEN_SYMBOL ) ;
        case 1 :
            return asset( string_to_price("2200.0000"), TOKEN_SYMBOL ) ;
        case 2 :
            return asset( string_to_price("2500.0000"), TOKEN_SYMBOL ) ;
        case 3 :
            return asset( string_to_price("10000.0000"), TOKEN_SYMBOL ) ;
        case 4 :
            return asset( string_to_price("2000.0000"), TOKEN_SYMBOL ) ;
        case 5 :
            return asset( string_to_price("1500.0000"), TOKEN_SYMBOL ) ;
        case 6 :
            return asset( string_to_price("1000.0000"), TOKEN_SYMBOL ) ;
        case 7 :
            return asset( string_to_price("1800.0000"), TOKEN_SYMBOL ) ;
        case 8 :
            return asset( string_to_price("3000.0000"), TOKEN_SYMBOL ) ;
        case 9 :
            return asset( string_to_price("1800.0000"), TOKEN_SYMBOL ) ;
        case 10 :
            return asset( string_to_price("10000.0000"), TOKEN_SYMBOL ) ;
        case 11 :
            return asset( string_to_price("1800.0000"), TOKEN_SYMBOL ) ;
        case 12 :
            return asset( string_to_price("1500.0000"), TOKEN_SYMBOL ) ;
        case 13 :
            return asset( string_to_price("1800.0000"), TOKEN_SYMBOL ) ;
        case 14 :
            return asset( string_to_price("2800.0000"), TOKEN_SYMBOL ) ;
        case 15 :
            return asset( string_to_price("1400.0000"), TOKEN_SYMBOL ) ;
        case 16 :
            return asset( string_to_price("1200.0000"), TOKEN_SYMBOL ) ;
        case 17 :
            return asset( string_to_price("1000.0000"), TOKEN_SYMBOL ) ;
        case 18 :
            return asset( string_to_price("1000.0000"), TOKEN_SYMBOL ) ;
        case 19 :
            return asset( string_to_price("1000.0000"), TOKEN_SYMBOL ) ;
        case 20 :
            return asset( string_to_price("1000.0000"), TOKEN_SYMBOL ) ;
        case 21 :
            return asset( string_to_price("1000.0000"), TOKEN_SYMBOL ) ;
        case 22 :
            return asset( string_to_price("20000.0000"), TOKEN_SYMBOL ) ;
        case 23 :
            return asset( string_to_price("14000.0000"), TOKEN_SYMBOL ) ;
        case 24 :
            return asset( string_to_price("12000.0000"), TOKEN_SYMBOL ) ;
        case 25 :
            return asset( string_to_price("4500.0000"), TOKEN_SYMBOL ) ;
        case 26 :
            return asset( string_to_price("4000.0000"), TOKEN_SYMBOL ) ;
        case 27 :
            return asset( string_to_price("2500.0000"), TOKEN_SYMBOL ) ;
        case 28 :
            return asset( string_to_price("0.0000"), EOS_SYMBOL ) ;
        default :
            return asset( string_to_price("0.0000"), TOKEN_SYMBOL ) ;
    }
}

/*
20190308 在合约里把游戏收益用于分红调成65%， 回购调成10% ，集齐整版纪念币奖励调成25%

剩余总量为429,600张时	挖矿成本为        1EOS进行一次挖矿
剩余总量为408,120张时	挖矿成本增加到1.100EOS进行一次挖矿
剩余总量为386,640张时	挖矿成本增加到1.210EOS进行一次挖矿
剩余总量为365,160张时	挖矿成本增加到1.331EOS进行一次挖矿
剩余总量为343,680张时	挖矿成本增加到1.464EOS进行一次挖矿
剩余总量为322,200张时	挖矿成本增加到1.611EOS进行一次挖矿
剩余总量为279,240张时	挖矿成本增加到1.772EOS进行一次挖矿
剩余总量为408,120张时	挖矿成本增加到1.949EOS进行一次挖矿
剩余总量为257,760张时	挖矿成本增加到2.144EOS进行一次挖矿
剩余总量为236,280张时	挖矿成本增加到2.358EOS进行一次挖矿
剩余总量为214,800张时	挖矿成本增加到2.594EOS进行一次挖矿
剩余总量为193,320张时	挖矿成本增加到2.853EOS进行一次挖矿
剩余总量为171,840张时	挖矿成本增加到3.138EOS进行一次挖矿
剩余总量为150,360张时	挖矿成本增加到3.452EOS进行一次挖矿
剩余总量为128,880张时	挖矿成本增加到3.797EOS进行一次挖矿
剩余总量为107,400张时	挖矿成本增加到4.177EOS进行一次挖矿
剩余总量为 85,920张时	挖矿成本增加到4.595EOS进行一次挖矿
剩余总量为 64,440张时	挖矿成本增加到5.054EOS进行一次挖矿
剩余总量为 42,960张时	挖矿成本增加到5.560EOS进行一次挖矿
剩余总量为 21,480张时	挖矿成本增加到6.116EOS进行一次挖矿
*/
/*
集齐整版比特币纪念币可获得 10000 通证奖励，即领即取
集齐整版以太币纪念币可获得 2200 通证奖励，即领即取
集齐整版莱特币纪念币可获得 2500 通证奖励，即领即取
集齐整版币安币纪念币可获得 10000 通证奖励，即领即取
集齐整版韭菜币纪念币可获得 2000 通证奖励，即领即取
集齐整版嫩模币纪念币可获得 1500 通证奖励，即领即取
集齐整版阿姨币纪念币可获得 1000 通证奖励，即领即取
集齐整版小蚁币纪念币可获得 1800 通证奖励，即领即取
集齐整版泰达币纪念币可获得 3000  通证奖励，即领即取
集齐整版石油币纪念币可获得 1800  通证奖励，即领即取
集齐整版门罗币纪念币可获得 10000 通证奖励，即领即取
集齐整版量子币纪念币可获得 1800  通证奖励，即领即取
集齐整版比原币纪念币可获得 1500  通证奖励，即领即取
集齐整版火币纪念币可获得   1800    通证奖励，即领即取
集齐整版EOS币纪念币可获得  2800  通证奖励，即领即取
集齐整版埃欧塔纪念币可获得 1400 通证奖励，即领即取
集齐整版中币纪念币可获得   1200    通证奖励，即领即取
集齐整版恒星币纪念币可获得 1000 通证奖励，即领即取
集齐整版艾达币纪念币可获得 1000 通证奖励，即领即取
集齐整版狗币纪念币可获得   1000   通证奖励，即领即取
集齐整版瑞波币纪念币可获得 1000 通证奖励，即领即取
集齐整版波场币纪念币可获得 1000  通证奖励，即领即取

集齐整版比特币是金、莱特币是银、门罗币是铜大额转账类整版纪念币（包括比特币整版、莱特币整版、门罗币整版）可获得20000 TOKEN奖励，即领即取；
                                                                0           2         10
集齐整版智能合约平台类纪念币（包括以太币整版、柚子币整版、小蚁币整版、量子币整版、比原币整版、阿姨币整版、埃欧塔币整版、艾达币整版、波场币整版）可获得14000 TOKEN奖励，即领即取；
                                 1        14         7           11        12          6         15           18           21     
集齐整版交易平台类纪念币（包括币安币整版、火币通证整版、中币整版）可获得12000 TOKEN奖励，即领即取；
                            3          13             16 
集齐整版支付类纪念币（包括恒星币整版、嫩模币整版、瑞波币整版）可获得4500 TOKEN奖励，即领即取；
                        17         5             20
即齐整版实物和法币锚定类纪念币（包括石油币整版、泰达币整版）可获得4000 TOKEN奖励，即领即取；
                                 9           8
即齐整版幽默类纪念币（包括狗币整版、韭菜币整版）可获得2500 TOKEN奖励，即领即取；
                        19       4
*/

} // namespace config

#endif