/**
 * Huffman 无损压缩算法 — 自实现
 *
 * 算法流程：
 * 1. 统计字符频率
 * 2. 用小顶堆（优先队列）建 Huffman 树
 * 3. 遍历树生成前缀码表
 * 4. 编码：原文每个字符替换为前缀码，按位打包
 * 5. 解码：逐位走 Huffman 树，到叶子输出字符
 *
 * 存储格式（JSON兼容）：
 *   { "freqs": {"a":5,"b":3,...}, "data": "...hex..." }
 *
 * 时间复杂度：O(n + m·log m)  n=文本长度, m=不同字符数
 */
#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <QString>
#include <QMap>
#include <QVector>
#include <QByteArray>
#include <QJsonObject>

// ============================================================
// Huffman 树节点
// ============================================================
struct HuffNode {
    QChar  ch;       // 字符（仅叶子节点有效）
    int    freq;     // 频率
    HuffNode* left;
    HuffNode* right;

    HuffNode(QChar c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
    HuffNode(int f, HuffNode* l, HuffNode* r) : ch(0), freq(f), left(l), right(r) {}
};

// ============================================================
// Huffman 压缩器
// ============================================================
class HuffmanCompressor {
public:
    // 压缩：文本 → {freqs, data_hex}
    static QJsonObject compress(const QString& text);

    // 解压：{freqs, data_hex} → 文本
    static QString decompress(const QJsonObject& packed);

    // 判断数据是否被压缩过
    static bool isCompressed(const QJsonObject& packed) {
        return packed.contains("freqs") && packed.contains("data");
    }

private:
    // 步骤1：统计频率
    static QMap<QChar, int> buildFreqMap(const QString& text);

    // 步骤2：用小顶堆建 Huffman 树
    static HuffNode* buildTree(const QMap<QChar, int>& freqs);

    // 步骤3：遍历树生成前缀码表
    static void buildCodeTable(HuffNode* node, const QString& prefix,
                                QMap<QChar, QString>& table);

    // 步骤4：编码文本 → 01字符串
    static QString encodeToBits(const QString& text,
                                 const QMap<QChar, QString>& table);

    // 步骤5：01字符串 → QByteArray（按位打包）
    static QByteArray packBits(const QString& bits);

    // 步骤6：QByteArray → hex字符串（JSON安全）
    static QString toHex(const QByteArray& data);

    // 解压步骤：
    static QByteArray fromHex(const QString& hex);
    static QString unpackBits(const QByteArray& data, int totalBits);
    static QString decodeFromBits(const QString& bits, HuffNode* root,
                                   int totalChars);

    // 步骤7：从频率表重建 Huffman 树
    static HuffNode* rebuildTree(const QJsonObject& freqs);

    // 删除树
    static void deleteTree(HuffNode* node);
};

// ============================================================
// 内联实现
// ============================================================

inline QMap<QChar, int> HuffmanCompressor::buildFreqMap(const QString& text)
{
    QMap<QChar, int> freqs;
    for (const QChar& c : text) {
        freqs[c]++;
    }
    return freqs;
}

// 小顶堆排序辅助
namespace {
void huffHeapPush(QVector<HuffNode*>& heap, HuffNode* node) {
    heap.append(node);
    int i = heap.size() - 1;
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (heap[parent]->freq <= heap[i]->freq) break;
        std::swap(heap[parent], heap[i]);
        i = parent;
    }
}

HuffNode* huffHeapPop(QVector<HuffNode*>& heap) {
    if (heap.isEmpty()) return nullptr;
    HuffNode* root = heap[0];
    heap[0] = heap.last();
    heap.removeLast();
    int i = 0;
    while (true) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;
        if (left < heap.size() && heap[left]->freq < heap[smallest]->freq)
            smallest = left;
        if (right < heap.size() && heap[right]->freq < heap[smallest]->freq)
            smallest = right;
        if (smallest == i) break;
        std::swap(heap[i], heap[smallest]);
        i = smallest;
    }
    return root;
}
} // anonymous namespace

inline HuffNode* HuffmanCompressor::buildTree(const QMap<QChar, int>& freqs)
{
    QVector<HuffNode*> heap;

    // 每个字符建一个叶子节点，推入小顶堆
    for (auto it = freqs.constBegin(); it != freqs.constEnd(); ++it) {
        huffHeapPush(heap, new HuffNode(it.key(), it.value()));
    }

    // 反复合并最小的两个节点，直到只剩一个根
    while (heap.size() > 1) {
        HuffNode* left  = huffHeapPop(heap);
        HuffNode* right = huffHeapPop(heap);
        huffHeapPush(heap, new HuffNode(left->freq + right->freq, left, right));
    }

    return heap.isEmpty() ? nullptr : heap[0];
}

inline void HuffmanCompressor::buildCodeTable(HuffNode* node,
                                                const QString& prefix,
                                                QMap<QChar, QString>& table)
{
    if (!node) return;
    // 叶子节点
    if (!node->left && !node->right) {
        table[node->ch] = prefix.isEmpty() ? "0" : prefix;  // 单字符特殊处理
        return;
    }
    buildCodeTable(node->left,  prefix + "0", table);
    buildCodeTable(node->right, prefix + "1", table);
}

inline QString HuffmanCompressor::encodeToBits(
    const QString& text, const QMap<QChar, QString>& table)
{
    QString bits;
    bits.reserve(text.size() * 4);  // 预估
    for (const QChar& c : text) {
        bits += table.value(c);
    }
    return bits;
}

inline QByteArray HuffmanCompressor::packBits(const QString& bits)
{
    QByteArray data;
    data.resize((bits.size() + 7) / 8);  // 向上取整到字节
    data.fill(0);

    for (int i = 0; i < bits.size(); ++i) {
        if (bits[i] == '1') {
            data[i / 8] |= (1 << (7 - (i % 8)));  // 大端序：高位在前
        }
    }
    return data;
}

inline QString HuffmanCompressor::toHex(const QByteArray& data)
{
    return QString::fromLatin1(data.toHex());
}

inline QJsonObject HuffmanCompressor::compress(const QString& text)
{
    if (text.isEmpty()) {
        QJsonObject empty;
        empty["freqs"] = QJsonObject();
        empty["data"] = "";
        return empty;
    }

    // 1. 统计频率
    QMap<QChar, int> freqs = buildFreqMap(text);

    // 2. 建 Huffman 树
    HuffNode* root = buildTree(freqs);

    // 3. 生成码表
    QMap<QChar, QString> codeTable;
    buildCodeTable(root, "", codeTable);

    // 4. 编码
    QString bits = encodeToBits(text, codeTable);

    // 5. 打包为字节
    QByteArray packed = packBits(bits);

    // 6. 输出 JSON
    QJsonObject result;
    QJsonObject freqObj;
    for (auto it = freqs.constBegin(); it != freqs.constEnd(); ++it) {
        freqObj[QString(it.key())] = it.value();
    }
    result["freqs"] = freqObj;
    result["data"]  = QString::fromLatin1(packed.toHex());
    result["len"]   = bits.size();  // 总比特数（用于解压时截断尾部填充位）

    deleteTree(root);
    return result;
}

inline QByteArray HuffmanCompressor::fromHex(const QString& hex)
{
    return QByteArray::fromHex(hex.toLatin1());
}

inline QString HuffmanCompressor::unpackBits(const QByteArray& data, int totalBits)
{
    QString bits;
    bits.reserve(totalBits);
    for (int i = 0; i < totalBits; ++i) {
        int byteIdx = i / 8;
        int bitIdx  = 7 - (i % 8);
        bits += ((data[byteIdx] >> bitIdx) & 1) ? '1' : '0';
    }
    return bits;
}

inline QString HuffmanCompressor::decodeFromBits(
    const QString& bits, HuffNode* root, int totalChars)
{
    if (!root) return "";
    QString text;
    text.reserve(totalChars);
    HuffNode* cur = root;

    for (int i = 0; i < bits.size() && totalChars > 0; ++i) {
        cur = (bits[i] == '0') ? cur->left : cur->right;
        if (cur && !cur->left && !cur->right) {
            // 叶子节点 → 输出字符
            text += cur->ch;
            cur = root;     // 回到根继续解码下一个字符
            totalChars--;
        }
    }
    return text;
}

inline HuffNode* HuffmanCompressor::rebuildTree(const QJsonObject& freqs)
{
    QMap<QChar, int> freqMap;
    for (auto it = freqs.constBegin(); it != freqs.constEnd(); ++it) {
        freqMap[it.key()[0]] = it.value().toInt();
    }
    return buildTree(freqMap);
}

inline QString HuffmanCompressor::decompress(const QJsonObject& packed)
{
    if (!isCompressed(packed)) return "";

    QJsonObject freqs = packed["freqs"].toObject();
    if (freqs.isEmpty()) return "";

    // 1. 重建 Huffman 树
    HuffNode* root = rebuildTree(freqs);

    // 2. 从 hex 取出字节
    QByteArray data = fromHex(packed["data"].toString());

    // 3. 还原为位串
    int totalBits = packed["len"].toInt();
    QString bits = unpackBits(data, totalBits);

    // 4. 解码：计算总字符数（频率总和）
    int totalChars = 0;
    for (auto it = freqs.constBegin(); it != freqs.constEnd(); ++it)
        totalChars += it.value().toInt();

    // 5. 遍历 Huffman 树解码
    QString text = "";
    if (root && totalChars > 0) {
        HuffNode* cur = root;
        for (int i = 0; i < bits.size() && totalChars > 0; ++i) {
            cur = (bits[i] == '0') ? cur->left : cur->right;
            if (cur && !cur->left && !cur->right) {
                text += cur->ch;
                totalChars--;
                cur = root;
            }
        }
    }

    deleteTree(root);
    return text;
}

inline void HuffmanCompressor::deleteTree(HuffNode* node)
{
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

#endif // HUFFMAN_H
