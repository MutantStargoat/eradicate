#include <stdio.h>
#include <string.h>
#include "cpuid.h"

static const char *cpuname(struct cpuid_info *cpu);
static const char *cpuvendor(struct cpuid_info *cpu);

struct cpuid_info cpuid;

void print_cpuid(struct cpuid_info *cpu)
{
	int i, col, len;
	static const char *featstr[32] = {
		"fpu", "vme", "dbgext", "pse", "tsc", "msr", "pae", "mce",
		"cx8", "apic", "?", "sep", "mtrr", "pge", "mca", "cmov",
		"pat", "pse36", "psn", "clf", "?", "dtes", "acpi", "mmx",
		"fxsr", "sse", "sse2", "ss", "htt", "tm1", "ia64", "pbe"};
	static const char *feat2str[32] = {
		"sse3", "pclmul", "dtes64", "monitor", "ds-cpl", "vmx", "smx", "est",
		"tm2", "ssse3", "cid", "sdbg", "fma", "cx16", "etprd", "pdcm",
		"?", "pcid", "dca", "sse41", "sse42", "x2apic", "movbe", "popcnt",
		"?", "aes", "xsave", "osxsave", "avx", "f16c", "rdrand", "?"};

	printf("CPU: %s - %s\n", cpuvendor(cpu), cpuname(cpu));
	printf("features:\n   ");
	col = 3;
	for(i=0; i<32; i++) {
		if(cpu->feat & (1 << i)) {
			len = strlen(featstr[i]) + 1;
			if(col + len >= 80) {
				fputs("\n   ", stdout);
				col = 3;
			}
			col += printf(" %s", featstr[i]);
		}
	}
	for(i=0; i<32; i++) {
		if(cpu->feat2 & (1 << i)) {
			len = strlen(feat2str[i]) + 1;
			if(col + len >= 80) {
				fputs("\n   ", stdout);
				col = 3;
			}
			col += printf(" %s", feat2str[i]);
		}
	}
	putchar('\n');
}

static const char *fam4_models[16] = {
	"486 DX 25/33", "486 DX 50", "486 SX", "486 DX/2", "486 SL", "486 SX/2",
	0, "486 DX/2-WB", "486 DX/4", "486 DX/4-WB"
};
static const char *fam5_models[16] = {
	"Pentium 60/66", "Pentium 60/66", "Pentium 75-200", "OverDrive", "Pentium MMX",
	0, 0, "Mobile Pentium 75-200", "Mobile Pentium MMX", "Quark"
};
static const char *fam6_models[16] = {
	"Pentium Pro", "Pentium Pro", 0, "Pentium 2", "Pentium 2", "Pentium 2",
	"Mobile Pentium 2", "Pentium 3", "Pentium 3", 0, "Pentium 3", "Pentium 3"
};


static const char *cpuname(struct cpuid_info *cpu)
{
	int model, family;
	char *rd, *wr;

	if(*cpu->brandstr) {
		/* unwank the string */
		rd = wr = cpu->brandstr;
		while(*rd) {
			if(rd[0] == '(' && rd[1] == 'T' && rd[2] == 'M' && rd[3] == ')')
				rd += 4;
			else if(rd[0] == '(' && rd[1] == 'R' && rd[2] == ')')
				rd += 3;
			if(rd != wr) *wr = *rd;
			wr++;
			rd++;
		}
		*wr = 0;
		return cpu->brandstr;
	}

	if(CPUID_EXTMODEL(cpu->id)) {
		/* processors new enough to have an extended model, should also provide
		 * a brand string. If we end up here, we don't know what it is
		 */
		return "unknown";
	}

	model = CPUID_MODEL(cpu->id);
	family = CPUID_FAMILY(cpu->id) | (CPUID_EXTFAMILY(cpu->id) << 4);

	switch(family) {
	case 3:	return "386";
	case 4:	return fam4_models[model] ? fam4_models[model] : "486";
	case 5: return fam5_models[model] ? fam5_models[model] : "Pentium";
	case 6: return fam6_models[model] ? fam6_models[model] : "unknown";
	case 15: return "Pentium 4";
	default:
		break;
	}
	return "unknown";
}

static const char *cpuvendor(struct cpuid_info *cpu)
{
	static char other[16];
	static const struct { const char *wank, *vendor; } unwanktab[] = {
		{"GenuineIntel", "intel"},
		{"AuthenticAMD", "AMD"},
		{"AMDisbetter!", "AMD"},
		{"CentaurHauls", "IDT"},
		{"CyrixInstead", "Cyrix"},
		{"TransmetaCPU", "Transmeta"},
		{"GenuineTMx86", "Transmeta"},
		{"Geode by NSC", "NatSemi"},
		{"NexGenDriven", "NexGen"},
		{"RiseRiseRise", "Rise"},
		{"SiS SiS SiS ", "SiS"},
		{"UMC UMC UMC ", "UMC"},
		{"VIA VIA VIA ", "VIA"},
		{"Vortex86 SoC", "DM&P"},
		{"  Shanghai  ", "Zhaoxin"},
		{"HygonGenuine", "Hygon"},
		{"E2K MACHINE", "MCST Elbrus"},
		{"MiSTer A0486", "ao486"},
		{"bhyve bhyve ", "bhyve"},
		{" KVMKVMKVM  ", "KVM"},
		{"TCGTCGTCGTCG", "qemu"},
		{"Microsoft Hv", "MS Hyper-V"},
		{" lrpepyh  vr", "Parallels"},
		{"VMwareVMware", "VMware"},
		{"XenVMMXenVMM", "Xen"},
		{"ACRNACRNACRN", "ACRN"},
		{" QNXQVMBSQG ", "QNX Hypervisor"},
		{0, 0}
	};

	int i;
	for(i=0; unwanktab[i].wank; i++) {
		if(memcmp(cpu->vendor, unwanktab[i].wank, 12) == 0) {
			return unwanktab[i].vendor;
		}
	}

	memcpy(other, cpu->vendor, 12);
	other[12] = 0;
	return other;
}
