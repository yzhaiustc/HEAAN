#include <NTL/ZZ.h>
#include <NTL/ZZX.h>
#include <iostream>
#include <vector>
#include <random>

#include "czz/CZZ.h"
#include "czz/CZZX.h"
#include "scheme/Cipher.h"
#include "scheme/Params.h"
#include "scheme/PubKey.h"
#include "scheme/Scheme.h"
#include "scheme/SchemeAlgo.h"
#include "scheme/SecKey.h"
#include "utils/NumUtils.h"
#include "utils/Ring2Utils.h"
#include "utils/TimeUtils.h"

using namespace std;
using namespace NTL;


void testDumb() {
	cout << "!!! START TEST DUMB !!!" << endl;

	//----------------------------
	TimeUtils timeutils;
	long logn = 13;
	long logl = 4;
	long logp = 30;
	long L = 5;
	double sigma = 3;
	double rho = 0.5;
	long h = 64;
	Params params(logn, logl, logp, L, sigma, rho, h);
	SecKey secretKey(params);
	PubKey publicKey(params, secretKey);
	Scheme scheme(params, secretKey, publicKey);
	SchemeAlgo algo(scheme);
	//----------------------------

	CZZ m = params.cksi.pows[params.logn][0];
	Cipher c = scheme.encrypt(m, scheme.params.p);
	ZZX p1 = c.c0.rx;
	ZZX p2 = c.c1.rx;
	long degree = params.n;
	ZZ mod = params.q;
	ZZ tmp1, tmp2;

	timeutils.start("1");
	ZZX p = p1 * p2;
	timeutils.stop("1");

	timeutils.start("2");
	p1.SetLength(degree);
	p.SetLength(2 * degree);
	for (long i = 0; i < degree; ++i) {
		tmp1 = p.rep[i];
		tmp2 = p.rep[i + degree];
		tmp1 %= mod;
		tmp2 %= mod;
//		Ring2Utils::truncate(tmp1, logMod);
//		Ring2Utils::truncate(tmp2, logMod);
		AddMod(p1.rep[i], tmp1, -tmp2, mod);
	}
	timeutils.stop("2");

	cout << "!!! END TEST DUMB !!!" << endl;
}

void testEncodeAll() {
	cout << "!!! START TEST ENCODE ALL !!!" << endl;

	//----------------------------
	TimeUtils timeutils;
	long logn = 13;
	long logl = 4;
	long logp = 30;
	long L = 5;
	double sigma = 3;
	double rho = 0.5;
	long h = 64;
	Params params(logn, logl, logp, L, sigma, rho, h);
	SecKey secretKey(params);
	PubKey publicKey(params, secretKey);
	Scheme scheme(params, secretKey, publicKey);
	SchemeAlgo algo(scheme);
	//----------------------------

	vector<CZZ> mvec;
	CZZ m;
	for (long i = 0; i < params.n; ++i) {
		m = params.cksi.pows[params.logn][i % 3];
		mvec.push_back(m);
	}

	vector<CZZ> fftInv = NumUtils::fftInv(mvec, params.cksi);
	vector<CZZ> fft = NumUtils::fft(fftInv, params.cksi);
	cout << "---------------" << endl;
	Cipher cipher = scheme.encryptAll(mvec, scheme.params.p);
	Cipher cipher2 = scheme.encrypt(params.cksi.pows[params.logn][1], scheme.params.p);
	vector<CZZ> dvec = scheme.decryptAll(cipher2);

	for (long i = 0; i < params.n; ++i) {
		cout << "---------------------" << endl;
		cout << "mi: " << i << " : " << mvec[i].toString() << endl;
		cout << "mm: " << i << " : " << params.cksi.pows[params.logn][1].toString() << endl;
		cout << "di: " << i << " : " << dvec[i].toString() << endl;
		cout << "---------------------" << endl;
	}
	cout << "!!! END TEST ENCODE ALL !!!" << endl;
}

void testEncode() {
	cout << "!!! START TEST ENCODE !!!" << endl;

	//----------------------------
	TimeUtils timeutils;
	long logn = 13;
	long logl = 1;
	long logp = 30;
	long L = 6;
	double sigma = 3;
	double rho = 0.5;
	long h = 64;
	Params params(logn, logl, logp, L, sigma, rho, h);
	SecKey secretKey(params);
	PubKey publicKey(params, secretKey);
	Scheme scheme(params, secretKey, publicKey);
	SchemeAlgo algo(scheme);
	//----------------------------

	long logSlots = 4;
	long slots = (1 << logSlots);

	CZZ m;
	vector<CZZ> mvec, dvec;

	for (long i = 0; i < slots; ++i) {
		m = params.cksi.pows[logSlots][i % 3];
		mvec.push_back(m);
	}

	cout << "---------------" << endl;
	Cipher cipher = scheme.encrypt(logSlots, mvec, scheme.params.p);
	dvec = scheme.decrypt(logSlots, cipher);
	cout << "---------------" << endl;

	for (long i = 0; i < slots; ++i) {
		cout << "---------------------" << endl;
		cout << "mi: " << i << " : " << mvec[i].toString() << endl;
		cout << "di: " << i << " : " << dvec[i].toString() << endl;
		cout << "---------------------" << endl;
	}

	cout << "!!! STOP TEST ENCODE !!!" << endl;
}

void testPow() {
	cout << "!!! START TEST POW !!!" << endl;

	//----------------------------
	TimeUtils timeutils;
	long logn = 15;
	long logl = 1;
	long logp = 56;
	long L = 11;
	double sigma = 3;
	double rho = 0.5;
	long h = 64;
	Params params(logn, logl, logp, L, sigma, rho, h);
	SecKey secretKey(params);
	PubKey publicKey(params, secretKey);
	Scheme scheme(params, secretKey, publicKey);
	SchemeAlgo algo(scheme);
	//----------------------------

	long logN = 5;
	long deg = 10;

	vector<CZZ> m2k, d2k;
	vector<Cipher> c2k;
	CZZ e, m, m2;

	m = params.cksi.pows[logN][2];
	m2k.push_back(m);
	for (long i = 0; i < deg; ++i) {
		m2 = (m2k[i] * m2k[i]) >> params.logp;
		m2k.push_back(m2);
	}

	Cipher c = scheme.encrypt(m, scheme.params.p);

	cout << "------------------" << endl;
	timeutils.start("Power of 2");
	algo.powerOf2(c2k, c, deg);
	timeutils.stop("Power of 2");
	cout << "------------------" << endl;

	scheme.decrypt(d2k, c2k);

	for (long i = 0; i < deg + 1; ++i) {
		e = m2k[i] - d2k[i];
		cout << "------------------" << endl;
		cout << "m: " << i << " " << m2k[i].toString() << endl;
		cout << "d: " << i << " " << d2k[i].toString() << endl;
		cout << "e: " << i << " " << e.toString() << endl;
//		cout << "eBnds: " << i << " " << c2k[i].eBnd << endl;
//		cout << "mBnds: " << i << " " << c2k[i].mBnd << endl;
		cout << "------------------" << endl;
	}

	cout << "!!! END TEST POW !!!" << endl;
}

void testProd2() {
	cout << "!!! START TEST PROD !!!" << endl;

	//----------------------------
	TimeUtils timeutils;
	long logn = 15;
	long logl = 3;
	long logp = 56;
	long L = 11;
	double sigma = 3;
	double rho = 0.5;
	long h = 64;
	Params params(logn, logl, logp, L, sigma, rho, h);
	SecKey secretKey(params);
	PubKey publicKey(params, secretKey);
	Scheme scheme(params, secretKey, publicKey);
	SchemeAlgo algo(scheme);
	//----------------------------

	long deg = 6;
	long size = 1 << deg;
	long logN = 5;

	CZZ m, e;
	vector<CZZ> ms;
	vector<vector<CZZ>> ms2k, ds2k;
	vector<Cipher> cs;
	vector<vector<Cipher>> cs2k;

	for (long i = 0; i < size; ++i) {
		m = params.cksi.pows[logN][i % 3];
		Cipher c = scheme.encrypt(m, scheme.params.p);
		ms.push_back(m);
		cs.push_back(c);
	}

	ms2k.push_back(ms);
	for (long i = 1; i < deg + 1; ++i) {
		vector<CZZ> m2k;
		long idx = 0;
		long m2ksize = ms2k[i-1].size();
		while(idx < m2ksize) {
			CZZ m2 = (ms2k[i - 1][idx] * ms2k[i - 1][idx+1]) >> params.logp;
			m2k.push_back(m2);
			idx += 2;
		}
		ms2k.push_back(m2k);
	}

	cout << "------------------" << endl;
	timeutils.start("Prod 2");
	algo.prod2(cs2k, cs, deg);
	timeutils.stop("Prod 2");
	cout << "------------------" << endl;

	for (long i = 0; i < deg + 1; ++i) {
		vector<CZZ> d2k;
		scheme.decrypt(d2k, cs2k[i]);
		ds2k.push_back(d2k);
	}

	for (long i = 0; i < deg + 1; ++i) {
		for (long j = 0; j < cs2k[i].size(); ++j) {
			e = ms2k[i][j] - ds2k[i][j];
			cout << "------------------" << endl;
			cout << "m: " << i << " " << j << " " << ms2k[i][j].toString() << endl;
			cout << "d: " << i << " " << j << " " << ds2k[i][j].toString() << endl;
			cout << "e: " << i << " " << j << " " << e.toString() << endl;
//			cout << "eBnds: " << i << " " << j << " " << cs2k[i][j].eBnd << endl;
//			cout << "mBnds: " << i << " " << j << " " << cs2k[i][j].mBnd << endl;
			cout << "------------------" << endl;
		}
	}
	cout << "!!! END TEST PROD !!!" << endl;
}


void testInv() {
	cout << "!!! START TEST INV !!!" << endl;

	//----------------------------
	TimeUtils timeutils;
	long logn = 13;
	long logl = 2;
	long logp = 25;
	long L = 6;
	double sigma = 3;
	double rho = 0.5;
	long h = 64;
	Params params(logn, logl, logp, L, sigma, rho, h);
	SecKey secretKey(params);
	PubKey publicKey(params, secretKey);
	Scheme scheme(params, secretKey, publicKey);
	SchemeAlgo algo(scheme);
	//----------------------------

	long error = 13;
	long r = 6;

	ZZ halfp;
	CZZ m, mbar, minv, e;
	vector<CZZ> d2k;
	vector<Cipher> c2k, v2k;

	mbar.r = RandomBits_ZZ(error);
	m.r = 1;
	m.r <<= scheme.params.logp;
	m.r -= mbar.r;
	minv.r = scheme.params.p * scheme.params.p / m.r;
	halfp = scheme.params.p / 2;

	cout << "------------------" << endl;
	timeutils.start("Encrypt c");
	Cipher c = scheme.encrypt(mbar, halfp);
	timeutils.stop("Encrypt c");
	cout << "------------------" << endl;

	cout << "------------------" << endl;
	timeutils.start("Inverse");
	algo.inverse(c2k, v2k, c, r);
	timeutils.stop("Inverse");
	cout << "------------------" << endl;

	scheme.decrypt(d2k, v2k);

	for (long i = 0; i < r-1; ++i) {
		e = minv - d2k[i];
		cout << "------------------" << endl;
		cout << "minv:  " << i << " " << minv.toString() << endl;
		cout << "ds:    " << i << " " << d2k[i].toString() << endl;
		cout << "es:    " << i << " " << e.toString() << endl;
//		cout << "eBnds: " << i << " " << v2k[i].eBnd << endl;
//		cout << "mBnds: " << i << " " << v2k[i].mBnd << endl;
		cout << "------------------" << endl;
	}

	cout << "!!! END TEST INV !!!" << endl;
}

void testFFT() {
	cout << "!!! START TEST FFT !!!" << endl;

	//----------------------------
	TimeUtils timeutils;
	long logn = 13;
	long logl = 4;
	long logp = 30;
	long L = 3;
	double sigma = 3;
	double rho = 0.5;
	long h = 64;
	Params params(logn, logl, logp, L, sigma, rho, h);
	SecKey secretKey(params);
	PubKey publicKey(params, secretKey);
	Scheme scheme(params, secretKey, publicKey);
	SchemeAlgo algo(scheme);
	//----------------------------

	long logN = 4;
	long N = 1 << logN;
	long deg = 5;

	CZZ zero, tmp;

	vector<CZZ> mp1, mp2, mpx;
	vector<CZZ> mfft1, mfft2, mfftx;
	vector<Cipher> cp1, cp2, cpx;
	vector<Cipher> cfft1, cfft2;
	vector<CZZ> dpx;

	for (long i = 0; i < deg; ++i) {
		mp1.push_back(params.cksi.pows[logN][i]);
		mp2.push_back(params.cksi.pows[logN][i]);
	}

	for (long i = deg; i < N; ++i) {
		mp1.push_back(zero);
		mp2.push_back(zero);
	}

	mfft1 = NumUtils::fft(mp1, params.cksi);
	mfft2 = NumUtils::fft(mp2, params.cksi);

	for (long i = 0; i < N; ++i) {
		tmp = (mfft1[i] * mfft2[i]) >> params.logp;
		mfftx.push_back(tmp);
	}

	mpx = NumUtils::fftInv(mfftx, params.cksi);

	cout << "------------------" << endl;
	timeutils.start("Encrypting polynomials");
	for (long i = 0; i < N; ++i) {
		Cipher c1 = scheme.encrypt(mp1[i], scheme.params.p);
		Cipher c2 = scheme.encrypt(mp2[i], scheme.params.p);
		cp1.push_back(c1);
		cp2.push_back(c2);
	}
	timeutils.stop("Encrypting polynomials");
	cout << "------------------" << endl;

	cout << "------------------" << endl;
	timeutils.start("cfft 1");
//	cfft1 = algo.fft(cp1);
	cfft1 = algo.fft(cp1);
	timeutils.stop("cfft 1");
	cout << "------------------" << endl;

	cout << "------------------" << endl;
	timeutils.start("cfft 2");
//	cfft2 = algo.fft(cp2);
	cfft2 = algo.fft(cp2);
	timeutils.stop("cfft 2");
	cout << "------------------" << endl;

	cout << "------------------" << endl;
	timeutils.start("mul fft");
	for (long i = 0; i < N; ++i) {
		scheme.multModSwitchAndEqual(cfft1[i], cfft2[i]);
	}
	timeutils.stop("mul fft");
	cout << "------------------" << endl;

	cout << "------------------" << endl;
	timeutils.start("cfftx inv");
//	cpx = algo.fftInv(cfft1);
	cpx = algo.fftInv(cfft1);
	timeutils.stop("cfftx inv");
	cout << "------------------" << endl;

	for (long i = 0; i < N; ++i) {
		vector<CZZ> tmpvec;
		dpx.push_back(scheme.decrypt(cpx[i]));
	}

	for (long i = 0; i < N; ++i) {
		cout << "----------------------" << endl;
		cout << i << " step: cpx = " << mpx[i].toString() << endl;
		cout << i << " step: dpx = " << dpx[i].toString() << endl;
		cout << "----------------------" << endl;
	}

	cout << "!!! END TEST FFT !!!" << endl;

}

int main() {
//	----------------------------
//	testDumb();
//	testEncodeAll();
//	testEncode();
//	testPow();
//	testProd2();
//	testInv();
	testFFT();
//	----------------------------

	return 0;
}
