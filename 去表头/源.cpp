#include "XlLib.h"

//判断第y行中间部分的像素是否相同
//忽略两端rate比例，只考虑中间1-2*rate部分
int keyline0(Mat &img, const int y, const float rate = 0.2) {
	const int cs = img.channels(), n = img.cols*cs;
	const int s1 = rate*n, s2 = n - s1;
	const uchar *p = img.ptr<uchar>(y) +s1, *q = p + cs;
	int s = 0;

	for (int i = s1; i < s2; ++i, ++p, ++q)
	{
		s += p[0];
		if (abs(p[0] - q[0])>4) return -1;
	}
	return s / (s2 - s1);
}

int cutFileTitle(CStr oriPath, CStr dstPath, int &ok) {
	//【1】计算相关变量
	//来原目录
	string srcFolder = XlFile::GetFolder(oriPath);
	//目标目录
	string dstFolder = srcFolder;
	if (dstPath.find('.') != string::npos)
		dstFolder = XlFile::GetFolder(dstPath);
	//目标文件名
	string dstName = XlFile::GetName(dstPath);
	if (dstName.find('.') == string::npos) {
		//如果仅输入类型名，则填充默认的“*.”前缀，补充为"*.jpg"
		dstName = "*." + dstName;
	}
	//获取原文件名
	vecS names, namesNE;
	XlFile::GetNames(oriPath, names);
	XlFile::GetNamesNE(oriPath, namesNE);
	//检查目标名中是否有%d（含%06d等扩展），获取格式修饰符fmt
	string fmt;
	size_t s1 = dstName.find('%');
	if (s1 != string::npos) {
		size_t s2 = dstName.find('d', s1 + 1);
		if (s2 != string::npos) {
			fmt = dstName.substr(s1, s2 - s1 + 1);
		}
	}
	//【2】批量去表头
	for (int i = 0; i < names.size(); ++i) {
		Mat img = imread(srcFolder + names[i]);
		int px, high;
		for (high = 20; high < 50; high++)
		{
			px = keyline0(img, high);
			if (px <= 250 && px >= 0) break;
		}
		high;
		if (px <= 250 && px >= 0)
		{
			++ok;
			img = img(Rect(0, high, img.cols, img.rows - high));
			//将目标文件名中的'*'替换为原文件名
			string newNameNE = replace(dstName, "*", namesNE[i]);
			//将'%d'替换为编号（从1开始）
			if (fmt != "") {
				char tmp[20];
				sprintf(tmp, _S(fmt), ok);
				newNameNE = replace(newNameNE, fmt, tmp);
			}
			//生成新文件名与判断是否删除旧文件
			//cout << dstFolder << "\n";
			imwrite(dstFolder + newNameNE, img);
		}
	}
	return names.size();
}


void help() {
	printf("使用方法：\n"
		"    cutTitle 参数1 参数2 \n"
		"    参数1：原图片目录加名字\n"
		"    参数2：目标格式或路径，可使用*和%%06d等修饰输出名\n"
		"    示例：cutTitle Input/*.png Output/%%04d.png\n\n");
}

int main(int argc, char** argv)
{
	if (argc < 2) {
		help();
		getchar();
		return 0;
	}
	int ok = 0;

	XlTimer tt("去表头");
	tt.Start();

	int n = cutFileTitle(argv[1], argv[2], ok);

	tt.Stop();
	float tol = tt.TimeInSeconds();
	printf("共有%d张图像，去表头成功%d张，去表头成功率%.2f%%,共用时%.2f秒；平均每张%.3f秒，每秒%.1f张\n\n",
		n, ok, ok*100.0 / n, tol, tol / n, n / tol);

	return 0;
}
