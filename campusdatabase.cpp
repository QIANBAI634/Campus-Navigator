/**
 * 200 个景区/校园数据
 *
 * 使用函数分块构建 QVector，避免 MinGW g++ 对大 initializer list 静默崩溃
 */
#include "campusdatabase.h"

const QVector<CampusInfo>& getAllCampuses()
{
    static QVector<CampusInfo> list;
    if (!list.isEmpty()) return list;
    list.reserve(200);

    // === 一、39 所 985 高校 ===
    // 北京(8)
    list.append({"北京大学",         CampusType::UNIVERSITY_985, "北京", "海淀区", 4.9, 4.8, 39.987, 116.305, "综合,人文,未名湖"});
    list.append({"清华大学",         CampusType::UNIVERSITY_985, "北京", "海淀区", 5.0, 4.9, 40.009, 116.326, "工科,综合,清华园"});
    list.append({"中国人民大学",     CampusType::UNIVERSITY_985, "北京", "海淀区", 4.6, 4.5, 39.970, 116.318, "人文社科,法学,经济学"});
    list.append({"北京航空航天大学", CampusType::UNIVERSITY_985, "北京", "海淀区", 4.5, 4.4, 39.982, 116.347, "工科,航天,航空"});
    list.append({"北京理工大学",     CampusType::UNIVERSITY_985, "北京", "海淀区", 4.4, 4.3, 39.960, 116.318, "工科,兵器,车辆"});
    list.append({"中国农业大学",     CampusType::UNIVERSITY_985, "北京", "海淀区", 4.3, 4.2, 40.005, 116.354, "农学,生命科学"});
    list.append({"北京师范大学",     CampusType::UNIVERSITY_985, "北京", "海淀区", 4.5, 4.4, 39.962, 116.366, "师范,教育,心理"});
    list.append({"中央民族大学",     CampusType::UNIVERSITY_985, "北京", "海淀区", 4.1, 4.0, 39.950, 116.318, "民族学,文科"});

    // 天津(2)
    list.append({"南开大学",         CampusType::UNIVERSITY_985, "天津", "南开区", 4.6, 4.5, 39.097, 117.174, "综合,化学,历史"});
    list.append({"天津大学",         CampusType::UNIVERSITY_985, "天津", "南开区", 4.5, 4.4, 39.103, 117.177, "工科,建筑,化工"});

    // 辽宁(2)
    list.append({"大连理工大学",     CampusType::UNIVERSITY_985, "辽宁", "大连市", 4.3, 4.2, 38.878, 121.530, "工科,化工,力学"});
    list.append({"东北大学",         CampusType::UNIVERSITY_985, "辽宁", "沈阳市", 4.2, 4.1, 41.763, 123.410, "工科,自动化,计算机"});

    // 吉林(1)
    list.append({"吉林大学",         CampusType::UNIVERSITY_985, "吉林", "长春市", 4.1, 4.0, 43.879, 125.303, "综合,汽车,法学"});

    // 黑龙江(1)
    list.append({"哈尔滨工业大学",   CampusType::UNIVERSITY_985, "黑龙江", "哈尔滨市", 4.6, 4.5, 45.743, 126.637, "工科,航天,机器人"});

    // 上海(4)
    list.append({"复旦大学",         CampusType::UNIVERSITY_985, "上海", "杨浦区", 4.8, 4.7, 31.297, 121.505, "综合,医学,新闻"});
    list.append({"上海交通大学",     CampusType::UNIVERSITY_985, "上海", "闵行区", 4.8, 4.7, 31.026, 121.437, "工科,医学,船舶"});
    list.append({"同济大学",         CampusType::UNIVERSITY_985, "上海", "杨浦区", 4.5, 4.4, 31.284, 121.502, "工科,建筑,土木"});
    list.append({"华东师范大学",     CampusType::UNIVERSITY_985, "上海", "普陀区", 4.3, 4.2, 31.231, 121.405, "师范,教育,地理"});

    // 江苏(2)
    list.append({"南京大学",         CampusType::UNIVERSITY_985, "江苏", "南京市", 4.7, 4.6, 32.060, 118.788, "综合,天文,地质"});
    list.append({"东南大学",         CampusType::UNIVERSITY_985, "江苏", "南京市", 4.4, 4.3, 32.058, 118.795, "工科,建筑,电子"});

    // 浙江(1)
    list.append({"浙江大学",         CampusType::UNIVERSITY_985, "浙江", "杭州市", 4.7, 4.6, 30.265, 120.128, "综合,工科,农学"});

    // 安徽(1)
    list.append({"中国科学技术大学", CampusType::UNIVERSITY_985, "安徽", "合肥市", 4.6, 4.5, 31.830, 117.277, "理工,物理,量子"});

    // 福建(1)
    list.append({"厦门大学",         CampusType::UNIVERSITY_985, "福建", "厦门市", 4.5, 4.4, 24.438, 118.090, "综合,海洋,会计"});

    // 山东(2)
    list.append({"山东大学",         CampusType::UNIVERSITY_985, "山东", "济南市", 4.2, 4.1, 36.670, 117.053, "综合,医学,文史"});
    list.append({"中国海洋大学",     CampusType::UNIVERSITY_985, "山东", "青岛市", 4.1, 4.0, 36.063, 120.383, "海洋,水产"});

    // 湖北(2)
    list.append({"武汉大学",         CampusType::UNIVERSITY_985, "湖北", "武汉市", 4.7, 4.6, 30.542, 114.363, "综合,法学,测绘,樱花"});
    list.append({"华中科技大学",     CampusType::UNIVERSITY_985, "湖北", "武汉市", 4.5, 4.4, 30.512, 114.418, "工科,医学,光电"});

    // 湖南(2)
    list.append({"湖南大学",         CampusType::UNIVERSITY_985, "湖南", "长沙市", 4.2, 4.1, 28.181, 112.954, "综合,土木,岳麓书院"});
    list.append({"中南大学",         CampusType::UNIVERSITY_985, "湖南", "长沙市", 4.3, 4.2, 28.166, 112.935, "工科,医学,材料"});

    // 国防科技大学
    list.append({"国防科技大学",     CampusType::UNIVERSITY_985, "湖南", "长沙市", 4.4, 4.3, 28.223, 112.992, "军事,计算机,航天"});

    // 广东(2)
    list.append({"中山大学",         CampusType::UNIVERSITY_985, "广东", "广州市", 4.5, 4.4, 23.099, 113.306, "综合,医学,岭南"});
    list.append({"华南理工大学",     CampusType::UNIVERSITY_985, "广东", "广州市", 4.3, 4.2, 23.153, 113.345, "工科,轻工,建筑"});

    // 四川(2)
    list.append({"四川大学",         CampusType::UNIVERSITY_985, "四川", "成都市", 4.3, 4.2, 30.633, 104.080, "综合,医学,口腔"});
    list.append({"电子科技大学",     CampusType::UNIVERSITY_985, "四川", "成都市", 4.4, 4.3, 30.753, 103.932, "电子,通信,计算机"});

    // 重庆(1)
    list.append({"重庆大学",         CampusType::UNIVERSITY_985, "重庆", "沙坪坝区", 4.2, 4.1, 29.571, 106.473, "工科,建筑,电气"});

    // 陕西(3)
    list.append({"西安交通大学",     CampusType::UNIVERSITY_985, "陕西", "西安市", 4.5, 4.4, 34.245, 108.997, "工科,管理,电气"});
    list.append({"西北工业大学",     CampusType::UNIVERSITY_985, "陕西", "西安市", 4.3, 4.2, 34.242, 108.914, "工科,航空,航天,航海"});
    list.append({"西北农林科技大学", CampusType::UNIVERSITY_985, "陕西", "咸阳市", 3.8, 3.7, 34.286, 108.071, "农林,葡萄酒"});

    // 甘肃(1)
    list.append({"兰州大学",         CampusType::UNIVERSITY_985, "甘肃", "兰州市", 4.0, 3.9, 36.048, 103.853, "综合,化学,草学"});

    // === 二、北京 211 高校（非985，18所）===
    list.append({"北京交通大学",     CampusType::UNIVERSITY_211, "北京", "海淀区", 4.2, 4.1, 39.952, 116.342, "交通,通信,管理"});
    list.append({"北京工业大学",     CampusType::UNIVERSITY_211, "北京", "朝阳区", 4.0, 3.9, 39.873, 116.486, "工科,材料,环能"});
    list.append({"北京科技大学",     CampusType::UNIVERSITY_211, "北京", "海淀区", 4.1, 4.0, 39.989, 116.356, "工科,钢铁,材料"});
    list.append({"北京化工大学",     CampusType::UNIVERSITY_211, "北京", "朝阳区", 3.9, 3.8, 39.974, 116.423, "化工,材料"});
    list.append({"北京邮电大学",     CampusType::UNIVERSITY_211, "北京", "海淀区", 4.3, 4.2, 40.157, 116.288, "通信,计算机,网安"});
    list.append({"北京林业大学",     CampusType::UNIVERSITY_211, "北京", "海淀区", 3.9, 3.8, 40.008, 116.348, "林业,园林,生态"});
    list.append({"北京中医药大学",   CampusType::UNIVERSITY_211, "北京", "朝阳区", 3.8, 3.7, 39.976, 116.419, "中医,中药,针灸"});
    list.append({"北京外国语大学",   CampusType::UNIVERSITY_211, "北京", "海淀区", 4.2, 4.1, 39.958, 116.308, "外语,翻译,国关"});
    list.append({"中国传媒大学",     CampusType::UNIVERSITY_211, "北京", "朝阳区", 4.4, 4.3, 39.914, 116.556, "传媒,播音,新闻"});
    list.append({"中央财经大学",     CampusType::UNIVERSITY_211, "北京", "海淀区", 4.3, 4.2, 39.962, 116.343, "财经,金融,会计"});
    list.append({"对外经济贸易大学", CampusType::UNIVERSITY_211, "北京", "朝阳区", 4.1, 4.0, 39.981, 116.430, "经贸,外语,法律"});
    list.append({"北京体育大学",     CampusType::UNIVERSITY_211, "北京", "海淀区", 3.9, 3.8, 40.024, 116.316, "体育,运动训练"});
    list.append({"中央音乐学院",     CampusType::UNIVERSITY_211, "北京", "西城区", 3.9, 3.8, 39.903, 116.360, "音乐,作曲,表演"});
    list.append({"中国政法大学",     CampusType::UNIVERSITY_211, "北京", "昌平区", 4.2, 4.1, 40.220, 116.246, "法学,政治学"});
    list.append({"华北电力大学",     CampusType::UNIVERSITY_211, "北京", "昌平区", 3.8, 3.7, 40.091, 116.301, "电力,能源,电气"});
    list.append({"中国矿业大学(北京)",CampusType::UNIVERSITY_211, "北京", "海淀区", 3.7, 3.6, 39.997, 116.349, "矿业,安全工程"});
    list.append({"中国石油大学(北京)",CampusType::UNIVERSITY_211, "北京", "昌平区", 3.8, 3.7, 40.217, 116.249, "石油,地质,化工"});
    list.append({"中国地质大学(北京)",CampusType::UNIVERSITY_211, "北京", "海淀区", 3.8, 3.7, 39.988, 116.348, "地质,资源,环境"});

    // === 三、北京著名景点(35) ===
    list.append({"故宫",             CampusType::SCENIC_SPOT, "北京", "东城区", 5.0, 4.9, 39.917, 116.397, "历史文化,皇家宫殿,世界遗产"});
    list.append({"天坛",             CampusType::SCENIC_SPOT, "北京", "东城区", 4.8, 4.7, 39.884, 116.415, "历史文化,祭祀,世界遗产"});
    list.append({"颐和园",           CampusType::SCENIC_SPOT, "北京", "海淀区", 4.9, 4.8, 39.997, 116.277, "皇家园林,昆明湖,世界遗产"});
    list.append({"圆明园",           CampusType::SCENIC_SPOT, "北京", "海淀区", 4.7, 4.6, 40.009, 116.303, "历史遗址,园林,爱国主义"});
    list.append({"八达岭长城",       CampusType::SCENIC_SPOT, "北京", "延庆区", 4.9, 4.8, 40.359, 116.019, "世界遗产,古代防御,登山"});
    list.append({"慕田峪长城",       CampusType::SCENIC_SPOT, "北京", "怀柔区", 4.7, 4.6, 40.435, 116.562, "长城,自然风光,徒步"});
    list.append({"天安门广场",       CampusType::SCENIC_SPOT, "北京", "东城区", 4.8, 4.7, 39.909, 116.397, "国家象征,升旗仪式,纪念碑"});
    list.append({"雍和宫",           CampusType::SCENIC_SPOT, "北京", "东城区", 4.6, 4.5, 39.948, 116.419, "藏传佛教,祈福,古建筑"});
    list.append({"恭王府",           CampusType::SCENIC_SPOT, "北京", "西城区", 4.6, 4.5, 39.938, 116.384, "清代王府,园林,和珅"});
    list.append({"明十三陵",         CampusType::SCENIC_SPOT, "北京", "昌平区", 4.4, 4.3, 40.288, 116.232, "明代皇陵,历史,神道"});
    list.append({"北海公园",         CampusType::SCENIC_SPOT, "北京", "西城区", 4.5, 4.4, 39.927, 116.389, "皇家园林,白塔,划船"});
    list.append({"景山公园",         CampusType::SCENIC_SPOT, "北京", "西城区", 4.5, 4.4, 39.926, 116.396, "观景,故宫全景,牡丹"});
    list.append({"什刹海",           CampusType::SCENIC_SPOT, "北京", "西城区", 4.5, 4.4, 39.939, 116.386, "胡同,酒吧,冰场"});
    list.append({"南锣鼓巷",         CampusType::SCENIC_SPOT, "北京", "东城区", 4.4, 4.3, 39.941, 116.407, "胡同,文创,小吃"});
    list.append({"798艺术区",        CampusType::SCENIC_SPOT, "北京", "朝阳区", 4.5, 4.4, 39.987, 116.497, "当代艺术,画廊,工业遗址"});
    list.append({"鸟巢(国家体育场)", CampusType::SCENIC_SPOT, "北京", "朝阳区", 4.6, 4.5, 39.994, 116.393, "奥运,建筑,演唱会"});
    list.append({"水立方",           CampusType::SCENIC_SPOT, "北京", "朝阳区", 4.4, 4.3, 39.993, 116.389, "奥运,水上运动,建筑"});
    list.append({"国家博物馆",       CampusType::SCENIC_SPOT, "北京", "东城区", 4.6, 4.5, 39.906, 116.403, "历史文物,展览,免费"});
    list.append({"首都博物馆",       CampusType::SCENIC_SPOT, "北京", "西城区", 4.3, 4.2, 39.907, 116.341, "北京历史,文物"});
    list.append({"古北水镇",         CampusType::SCENIC_SPOT, "北京", "密云区", 4.6, 4.5, 40.652, 117.249, "古镇,温泉,长城夜景"});
    list.append({"周口店遗址",       CampusType::SCENIC_SPOT, "北京", "房山区", 4.2, 4.1, 39.694, 115.943, "北京猿人,世界遗产"});
    list.append({"卢沟桥",           CampusType::SCENIC_SPOT, "北京", "丰台区", 4.3, 4.2, 39.853, 116.222, "抗战纪念,石狮,历史"});
    list.append({"潭柘寺",           CampusType::SCENIC_SPOT, "北京", "门头沟区", 4.3, 4.2, 39.904, 116.038, "古刹,银杏,祈福"});
    list.append({"大观园",           CampusType::SCENIC_SPOT, "北京", "西城区", 4.1, 4.0, 39.875, 116.358, "红楼梦,园林"});
    list.append({"国子监",           CampusType::SCENIC_SPOT, "北京", "东城区", 4.3, 4.2, 39.947, 116.419, "古代最高学府,科举"});
    list.append({"中华世纪坛",       CampusType::SCENIC_SPOT, "北京", "海淀区", 4.0, 3.9, 39.913, 116.332, "地标建筑,展览"});
    list.append({"国家大剧院",       CampusType::SCENIC_SPOT, "北京", "西城区", 4.4, 4.3, 39.908, 116.387, "表演艺术,建筑,歌剧"});
    list.append({"中央电视台",       CampusType::SCENIC_SPOT, "北京", "朝阳区", 4.2, 4.1, 39.914, 116.467, "地标建筑,传媒"});
    list.append({"欢乐谷",           CampusType::SCENIC_SPOT, "北京", "朝阳区", 4.3, 4.2, 39.871, 116.495, "游乐园,过山车,亲子"});
    list.append({"环球影城",         CampusType::SCENIC_SPOT, "北京", "通州区", 4.8, 4.7, 39.830, 116.688, "主题乐园,哈利波特,变形金刚"});
    list.append({"雁栖湖",           CampusType::SCENIC_SPOT, "北京", "怀柔区", 4.2, 4.1, 40.410, 116.670, "湖景,APEC,水上游乐"});
    list.append({"十渡",             CampusType::SCENIC_SPOT, "北京", "房山区", 4.1, 4.0, 39.633, 115.587, "峡谷,漂流,喀斯特地貌"});
    list.append({"龙庆峡",           CampusType::SCENIC_SPOT, "北京", "延庆区", 4.2, 4.1, 40.543, 116.066, "峡谷,冰灯,划船"});
    list.append({"大栅栏",           CampusType::SCENIC_SPOT, "北京", "西城区", 4.2, 4.1, 39.896, 116.397, "老字号,商业街,古建筑"});
    list.append({"簋街",             CampusType::SCENIC_SPOT, "北京", "东城区", 4.2, 4.1, 39.937, 116.434, "美食街,麻辣小龙虾,夜宵"});
    list.append({"前门大街",         CampusType::SCENIC_SPOT, "北京", "东城区", 4.2, 4.1, 39.899, 116.401, "老北京,商业街,铛铛车"});
    list.append({"王府井小吃街",     CampusType::SCENIC_SPOT, "北京", "东城区", 4.0, 3.9, 39.917, 116.415, "小吃,夜市,旅游"});
    list.append({"五道营胡同",       CampusType::SCENIC_SPOT, "北京", "东城区", 4.1, 4.0, 39.946, 116.414, "文艺,小店,咖啡馆"});
    list.append({"鼓楼",             CampusType::SCENIC_SPOT, "北京", "东城区", 4.2, 4.1, 39.941, 116.394, "古建筑,钟鼓文化,地标"});
    list.append({"烟袋斜街",         CampusType::SCENIC_SPOT, "北京", "西城区", 4.0, 3.9, 39.940, 116.391, "胡同,文创,后海"});
    list.append({"琉璃厂",           CampusType::SCENIC_SPOT, "北京", "西城区", 3.9, 3.8, 39.897, 116.384, "文房四宝,字画,古玩"});
    list.append({"后海酒吧街",       CampusType::SCENIC_SPOT, "北京", "西城区", 4.2, 4.1, 39.940, 116.388, "酒吧,音乐,夜景"});
    list.append({"戒台寺",           CampusType::SCENIC_SPOT, "北京", "门头沟区", 3.9, 3.8, 39.885, 116.080, "古刹,戒坛,古松"});
    list.append({"云居寺",           CampusType::SCENIC_SPOT, "北京", "房山区", 3.8, 3.7, 39.611, 115.759, "石经,古刹,佛教圣地"});

    // === 四、北京公园(25) ===
    list.append({"奥林匹克森林公园", CampusType::PARK, "北京", "朝阳区", 4.5, 4.4, 40.021, 116.391, "森林,跑步,野餐,免费"});
    list.append({"朝阳公园",         CampusType::PARK, "北京", "朝阳区", 4.3, 4.2, 39.942, 116.481, "湖景,摩天轮,音乐节"});
    list.append({"玉渊潭公园",       CampusType::PARK, "北京", "海淀区", 4.5, 4.4, 39.919, 116.318, "樱花,划船,电视塔"});
    list.append({"香山公园",         CampusType::PARK, "北京", "海淀区", 4.6, 4.5, 39.997, 116.190, "红叶,登山,古刹"});
    list.append({"北京动物园",       CampusType::PARK, "北京", "西城区", 4.4, 4.3, 39.943, 116.335, "动物,大熊猫,亲子"});
    list.append({"北京植物园",       CampusType::PARK, "北京", "海淀区", 4.3, 4.2, 40.003, 116.213, "植物,花展,温室"});
    list.append({"中山公园",         CampusType::PARK, "北京", "东城区", 4.2, 4.1, 39.912, 116.394, "郁金香,社稷坛,古树"});
    list.append({"地坛公园",         CampusType::PARK, "北京", "东城区", 4.1, 4.0, 39.955, 116.419, "古坛,银杏,庙会"});
    list.append({"陶然亭公园",       CampusType::PARK, "北京", "西城区", 4.0, 3.9, 39.875, 116.374, "湖亭,革命纪念,休闲"});
    list.append({"紫竹院公园",       CampusType::PARK, "北京", "海淀区", 4.1, 4.0, 39.947, 116.326, "竹林,划船,免费"});
    list.append({"龙潭公园",         CampusType::PARK, "北京", "东城区", 4.0, 3.9, 39.882, 116.445, "龙文化,春节庙会"});
    list.append({"世界公园",         CampusType::PARK, "北京", "丰台区", 3.9, 3.8, 39.816, 116.290, "微缩景观,世界建筑"});
    list.append({"八大处公园",       CampusType::PARK, "北京", "石景山区", 4.2, 4.1, 39.956, 116.194, "古刹,登山,佛牙舍利"});
    list.append({"西山国家森林公园", CampusType::PARK, "北京", "海淀区", 4.1, 4.0, 39.977, 116.199, "森林,登山,红叶"});
    list.append({"红螺寺",           CampusType::PARK, "北京", "怀柔区", 4.2, 4.1, 40.318, 116.636, "古刹,雌雄银杏,祈福"});
    list.append({"大运河森林公园",   CampusType::PARK, "北京", "通州区", 4.0, 3.9, 39.884, 116.726, "运河,骑行,露营"});
    list.append({"中华民族园",       CampusType::PARK, "北京", "朝阳区", 3.8, 3.7, 39.989, 116.393, "民族文化,建筑,歌舞"});
    list.append({"红领巾公园",       CampusType::PARK, "北京", "朝阳区", 3.6, 3.5, 39.931, 116.498, "社区公园,休闲"});
    list.append({"团结湖公园",       CampusType::PARK, "北京", "朝阳区", 3.6, 3.5, 39.934, 116.469, "湖景,休闲"});
    list.append({"海淀公园",         CampusType::PARK, "北京", "海淀区", 3.8, 3.7, 39.985, 116.293, "科技主题,音乐节"});
    list.append({"莲花池公园",       CampusType::PARK, "北京", "丰台区", 3.7, 3.6, 39.898, 116.321, "荷花,历史"});
    list.append({"北宫国家森林公园", CampusType::PARK, "北京", "丰台区", 3.8, 3.7, 39.855, 116.145, "森林,登山,红叶"});
    list.append({"蟒山国家森林公园", CampusType::PARK, "北京", "昌平区", 3.9, 3.8, 40.260, 116.284, "登山,水库,石佛"});
    list.append({"云蒙山",           CampusType::PARK, "北京", "密云区", 4.0, 3.9, 40.583, 116.745, "登山,瀑布,原始森林"});
    list.append({"百花山",           CampusType::PARK, "北京", "门头沟区", 3.9, 3.8, 39.857, 115.610, "高山草甸,野花,登山"});
    list.append({"八达岭野生动物园", CampusType::PARK, "北京", "延庆区", 3.9, 3.8, 40.361, 116.014, "野生动物,自驾游览"});
    list.append({"金海湖",           CampusType::PARK, "北京", "平谷区", 3.8, 3.7, 40.191, 117.320, "水上运动,露营,蹦极"});
    list.append({"汉石桥湿地公园",   CampusType::PARK, "北京", "顺义区", 3.6, 3.5, 40.119, 116.817, "湿地,观鸟,芦苇"});
    list.append({"野鸭湖",           CampusType::PARK, "北京", "延庆区", 3.8, 3.7, 40.426, 115.867, "湿地,候鸟,观鸟"});

    // === 五、北京医院(20) ===
    list.append({"协和医院",         CampusType::HOSPITAL, "北京", "东城区", 4.8, 4.7, 39.916, 116.418, "综合,疑难杂症,全国顶尖"});
    list.append({"301医院(解放军总医院)",CampusType::HOSPITAL, "北京", "海淀区", 4.7, 4.6, 39.931, 116.278, "综合,部队医院"});
    list.append({"中日友好医院",     CampusType::HOSPITAL, "北京", "朝阳区", 4.3, 4.2, 39.979, 116.438, "综合,中日合作"});
    list.append({"北京安贞医院",     CampusType::HOSPITAL, "北京", "朝阳区", 4.3, 4.2, 39.975, 116.419, "心血管,心脏专科"});
    list.append({"阜外医院",         CampusType::HOSPITAL, "北京", "西城区", 4.5, 4.4, 39.926, 116.353, "心血管,心脏病,全国顶尖"});
    list.append({"北京大学第一医院", CampusType::HOSPITAL, "北京", "西城区", 4.4, 4.3, 39.933, 116.371, "综合,妇产,儿科"});
    list.append({"北京大学人民医院", CampusType::HOSPITAL, "北京", "西城区", 4.3, 4.2, 39.935, 116.359, "综合,血液科"});
    list.append({"北医三院",         CampusType::HOSPITAL, "北京", "海淀区", 4.4, 4.3, 39.989, 116.351, "综合,骨科,生殖医学"});
    list.append({"同仁医院",         CampusType::HOSPITAL, "北京", "东城区", 4.3, 4.2, 39.893, 116.415, "眼科,耳鼻喉,全国顶尖"});
    list.append({"宣武医院",         CampusType::HOSPITAL, "北京", "西城区", 4.1, 4.0, 39.894, 116.363, "神经内科,老年医学"});
    list.append({"友谊医院",         CampusType::HOSPITAL, "北京", "西城区", 4.0, 3.9, 39.882, 116.379, "综合,消化"});
    list.append({"朝阳医院",         CampusType::HOSPITAL, "北京", "朝阳区", 4.0, 3.9, 39.929, 116.458, "综合,呼吸,急诊"});
    list.append({"天坛医院",         CampusType::HOSPITAL, "北京", "丰台区", 4.2, 4.1, 39.856, 116.423, "神经外科,脑科,全国顶尖"});
    list.append({"积水潭医院",       CampusType::HOSPITAL, "北京", "西城区", 4.3, 4.2, 39.949, 116.373, "骨科,烧伤,全国顶尖"});
    list.append({"北京儿童医院",     CampusType::HOSPITAL, "北京", "西城区", 4.2, 4.1, 39.917, 116.352, "儿科,全国顶尖"});
    list.append({"北京中医医院",     CampusType::HOSPITAL, "北京", "东城区", 4.0, 3.9, 39.942, 116.408, "中医,针灸,皮肤科"});
    list.append({"广安门医院",       CampusType::HOSPITAL, "北京", "西城区", 4.0, 3.9, 39.894, 116.357, "中医,肿瘤"});
    list.append({"北京妇产医院",     CampusType::HOSPITAL, "北京", "朝阳区", 4.0, 3.9, 39.917, 116.436, "妇产,生殖"});
    list.append({"北京口腔医院",     CampusType::HOSPITAL, "北京", "东城区", 3.9, 3.8, 39.892, 116.411, "口腔,牙科"});
    list.append({"北大口腔医院",     CampusType::HOSPITAL, "北京", "海淀区", 4.3, 4.2, 39.958, 116.334, "口腔,全国顶尖"});

    // === 六、北京商场(28) ===
    list.append({"三里屯太古里",     CampusType::MALL, "北京", "朝阳区", 4.7, 4.6, 39.936, 116.458, "时尚,潮牌,酒吧,餐饮"});
    list.append({"国贸商城",         CampusType::MALL, "北京", "朝阳区", 4.5, 4.4, 39.912, 116.465, "高端,奢侈品,商务"});
    list.append({"SKP",              CampusType::MALL, "北京", "朝阳区", 4.6, 4.5, 39.915, 116.474, "奢侈品,高端购物"});
    list.append({"王府井百货",       CampusType::MALL, "北京", "东城区", 4.3, 4.2, 39.916, 116.414, "老牌商场,旅游购物"});
    list.append({"西单大悦城",       CampusType::MALL, "北京", "西城区", 4.4, 4.3, 39.913, 116.378, "时尚,年轻人,餐饮"});
    list.append({"世贸天阶",         CampusType::MALL, "北京", "朝阳区", 4.2, 4.1, 39.919, 116.466, "天幕,时尚,休闲"});
    list.append({"蓝色港湾",         CampusType::MALL, "北京", "朝阳区", 4.3, 4.2, 39.953, 116.475, "欧式小镇,亲子,湖畔"});
    list.append({"朝阳大悦城",       CampusType::MALL, "北京", "朝阳区", 4.4, 4.3, 39.924, 116.524, "餐饮,影院,购物"});
    list.append({"颐堤港",           CampusType::MALL, "北京", "朝阳区", 4.1, 4.0, 39.972, 116.498, "休闲,超市,亲子"});
    list.append({"华熙LIVE",         CampusType::MALL, "北京", "海淀区", 4.2, 4.1, 39.916, 116.286, "演唱会,餐饮,电竞"});
    list.append({"金融街购物中心",   CampusType::MALL, "北京", "西城区", 4.0, 3.9, 39.922, 116.360, "高端,商务"});
    list.append({"北京APM",          CampusType::MALL, "北京", "东城区", 4.1, 4.0, 39.916, 116.416, "时尚,餐饮,影院"});
    list.append({"东方新天地",       CampusType::MALL, "北京", "东城区", 4.1, 4.0, 39.914, 116.420, "综合,商务"});
    list.append({"老佛爷百货",       CampusType::MALL, "北京", "西城区", 4.0, 3.9, 39.918, 116.377, "法式,轻奢,时尚"});
    list.append({"凯德MALL(西直门)", CampusType::MALL, "北京", "西城区", 3.9, 3.8, 39.943, 116.354, "综合,餐饮,超市"});
    list.append({"荟聚",             CampusType::MALL, "北京", "大兴区", 4.2, 4.1, 39.793, 116.336, "宜家,大而全,亲子"});
    list.append({"通州万达广场",     CampusType::MALL, "北京", "通州区", 4.0, 3.9, 39.906, 116.663, "综合,影院,餐饮"});
    list.append({"龙湖长楹天街",     CampusType::MALL, "北京", "朝阳区", 3.9, 3.8, 39.921, 116.602, "综合,餐饮,亲子"});
    list.append({"新中关购物中心",   CampusType::MALL, "北京", "海淀区", 3.9, 3.8, 39.988, 116.316, "科技园附近,餐饮"});
    list.append({"北京来福士",       CampusType::MALL, "北京", "东城区", 4.0, 3.9, 39.941, 116.436, "时尚,餐饮"});
    list.append({"金源新燕莎MALL",  CampusType::MALL, "北京", "海淀区", 3.8, 3.7, 39.962, 116.289, "大体量,亲子,餐饮"});
    list.append({"合生汇",           CampusType::MALL, "北京", "朝阳区", 4.2, 4.1, 39.907, 116.513, "餐饮,潮流,深夜食堂"});
    list.append({"崇文门新世界",     CampusType::MALL, "北京", "东城区", 3.8, 3.7, 39.902, 116.422, "综合,青春时尚"});
    list.append({"凤凰汇",           CampusType::MALL, "北京", "朝阳区", 3.8, 3.7, 39.971, 116.459, "社区,餐饮"});
    list.append({"望京凯德MALL",    CampusType::MALL, "北京", "朝阳区", 3.9, 3.8, 39.994, 116.481, "综合,餐饮,超市"});
    list.append({"动物园批发市场",   CampusType::MALL, "北京", "西城区", 3.8, 3.7, 39.943, 116.340, "批发,服装,便宜"});

    // === 七、文化场所 & 交通枢纽(18) ===
    list.append({"中国美术馆",       CampusType::CULTURE, "北京", "东城区", 4.3, 4.2, 39.927, 116.410, "美术展览,油画,国画"});
    list.append({"国家图书馆",       CampusType::CULTURE, "北京", "海淀区", 4.2, 4.1, 39.948, 116.326, "藏书,自习,阅览"});
    list.append({"北京天文馆",       CampusType::CULTURE, "北京", "西城区", 4.1, 4.0, 39.941, 116.335, "天文,科普,球幕影院"});
    list.append({"自然博物馆",       CampusType::CULTURE, "北京", "东城区", 4.0, 3.9, 39.895, 116.405, "恐龙,自然,科普"});
    list.append({"军事博物馆",       CampusType::CULTURE, "北京", "海淀区", 4.1, 4.0, 39.916, 116.324, "兵器,军史,坦克"});
    list.append({"中国科技馆",       CampusType::CULTURE, "北京", "朝阳区", 4.3, 4.2, 39.996, 116.398, "科学,互动,儿童"});
    list.append({"中国电影博物馆",   CampusType::CULTURE, "北京", "朝阳区", 3.9, 3.8, 39.993, 116.512, "电影历史,IMAX"});
    list.append({"汽车博物馆",       CampusType::CULTURE, "北京", "丰台区", 3.8, 3.7, 39.823, 116.309, "汽车,老爷车,互动"});
    list.append({"潘家园旧货市场",   CampusType::CULTURE, "北京", "朝阳区", 4.1, 4.0, 39.876, 116.462, "古玩,文玩,淘宝"});
    list.append({"北京首都国际机场", CampusType::TRANSPORT, "北京", "顺义区", 4.3, 4.2, 40.079, 116.599, "机场,T3航站楼"});
    list.append({"北京大兴国际机场", CampusType::TRANSPORT, "北京", "大兴区", 4.5, 4.4, 39.511, 116.422, "机场,凤凰展翅,新国门"});
    list.append({"北京站",           CampusType::TRANSPORT, "北京", "东城区", 3.8, 3.7, 39.905, 116.428, "火车站,京沪线"});
    list.append({"北京西站",         CampusType::TRANSPORT, "北京", "丰台区", 3.8, 3.7, 39.896, 116.323, "火车站,京广线"});
    list.append({"北京南站",         CampusType::TRANSPORT, "北京", "丰台区", 4.0, 3.9, 39.866, 116.381, "火车站,高铁"});
    list.append({"北京北站",         CampusType::TRANSPORT, "北京", "西城区", 3.5, 3.4, 39.946, 116.351, "火车站,S2线"});
    list.append({"北京朝阳站",       CampusType::TRANSPORT, "北京", "朝阳区", 3.6, 3.5, 39.921, 116.540, "火车站,东北方向"});
    list.append({"清河站",           CampusType::TRANSPORT, "北京", "海淀区", 3.5, 3.4, 40.042, 116.326, "火车站,京张高铁"});
    list.append({"四惠交通枢纽",     CampusType::TRANSPORT, "北京", "朝阳区", 3.4, 3.3, 39.907, 116.502, "公交枢纽,长途汽车"});
    list.append({"丰台站",           CampusType::TRANSPORT, "北京", "丰台区", 3.5, 3.4, 39.855, 116.310, "火车站,亚洲最大"});
    list.append({"鸟巢文化中心",     CampusType::CULTURE, "北京", "朝阳区", 3.9, 3.8, 39.995, 116.394, "文创,展览"});
    list.append({"北京展览馆",       CampusType::CULTURE, "北京", "西城区", 3.8, 3.7, 39.946, 116.348, "展览,俄罗斯建筑"});
    list.append({"国家会议中心",     CampusType::CULTURE, "北京", "朝阳区", 3.8, 3.7, 40.002, 116.390, "会议,展览"});
    list.append({"北京规划展览馆",   CampusType::CULTURE, "北京", "东城区", 3.7, 3.6, 39.901, 116.402, "城市规划,沙盘"});
    list.append({"中国地质博物馆",   CampusType::CULTURE, "北京", "西城区", 3.7, 3.6, 39.925, 116.367, "矿石,宝石,化石"});

    return list;
}
