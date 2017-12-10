#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>

enum unit_t {
	unit_err = 0,
	unit_grams,
	unit_kilograms,
	unit_liters,
	unit_mililiters
};

/* lists interface between units*/
enum cutoff_t {
	cutoff_grams_kilograms = 500,
	cutoff_mililiters_liters = 500,
};

int valid_conversion(enum unit_t n1, enum unit_t n2)
{
	switch(n1) {
	case unit_grams:
	case unit_kilograms:
		switch(n2) {
		case unit_grams:
		case unit_kilograms:
			return 1;
		case unit_liters:
		case unit_mililiters:
			return 0;
		default:
			fprintf(stderr, "Unknown unit_t.\n");
			assert(0);
		}
	case unit_liters:
	case unit_mililiters:
		switch(n2) {
		case unit_liters:
		case unit_mililiters:
			return 1;
		case unit_grams:
		case unit_kilograms:
			return 0;
		default:
			fprintf(stderr, "Unknown unit_t.\n");
			assert(0);
		}
	default:
		fprintf(stderr, "Unknown unit_t.\n");
		assert(0);
	}

	fprintf(stderr, "Unintended fallthrough.\n");
	assert(0);
}

void skip_nondigit(char **s)
{
	if(!s)
		return;
	if(!*s)
		return;

	while(!isdigit(**s) && **s)
		(*s)++;
}

void str_tolower(char *str)
{
	while(*str) {
		*str = tolower(*str);
		++str;
	}
}
int x2x(long double *num, enum unit_t u_in, enum unit_t u_out)
{
	if(!valid_conversion(u_in, u_out)) {
		fprintf(stderr, "Invalid conversion.\n");
		return 0;
	}

	long double factor = 1;

	/* weight */
	switch(u_in) {
	case unit_grams:
		switch(u_out) {
		case unit_grams: break;

		case unit_kilograms:
			factor = 0.001;
			break;
		default:
			goto assert_factor;
		}
		break;
	case unit_kilograms:
		switch(u_out) {
		case unit_kilograms: break;

		case unit_grams:
			factor = 1000;
			break;
		default:
			goto assert_factor;
		}
		break;

	/* volume */
	case unit_liters:
		switch(u_out) {
		case unit_liters: break;

		case unit_mililiters:
			factor = 1000;
			break;
		default:
			goto assert_factor;
		}
		break;
	case unit_mililiters:
		switch(u_out) {
		case unit_mililiters: break;

		case unit_liters:
			factor = .001;
			break;
		default:
			goto assert_factor;
		}
		break;
	default:
		goto assert_factor;
	}

	*num = *num * factor;
	return 1;

assert_factor:
	fprintf(stderr, "No conversion factor found.\n");
	assert(0);
	return 0;
}

enum unit_t detect_units(char *str)
{
	if(strstr(str, "kg"))
		return unit_kilograms;
	if(strstr(str, "g"))
		return unit_grams;
	if(strstr(str, "ml"))
		return unit_mililiters;
	if(strstr(str, "l"))
		return unit_liters;

	fprintf(stderr, "Could not find unit: \"%s\"\n", str);
	return unit_err;
}
char *pname;

void usage(int e)
{
	fprintf(stderr, "usage: %s measure new_unit [measure new_unit] ...\n", pname);
	exit(e);
}
const int max_str_len = 32;
int value_and_unit(const char *const str, long double *num, enum unit_t *u)
{
	char str_tmp[max_str_len];
	char *str_p = str_tmp;
	char **str_pp = &str_p;

	strncpy(str_p, str, max_str_len);
	str_tolower(str_p);
	*u = detect_units(str_p);
	skip_nondigit(str_pp);
	if(!*str_p) {
		fprintf(stderr, "Could not find number: \"%s\"\n", str);
		return 0;
	}
	char *endptr;
	char **endptr_p = &endptr;
	*num = strtold(str_p, endptr_p);
	if(endptr_p) { /* when strtold found a number */
		if(*endptr_p == str_p) { /* didn't convert it though */
			fprintf(stderr, "Could not convert: \"%s\"\n", str);
			return 0;
		}
	} else {
		fprintf(stderr, "Could not find number: \"%s\"\n", str);
		return 0;
	}

	return 1;
}
/*returns 1 on sucess
 * TODO: This function belongs in a library to be used from android and linux. */
int convert(char *measure, char *new_unit, long double *num)
{
	enum unit_t from_unit;
	enum unit_t to_unit = detect_units(new_unit);
	if(!value_and_unit(measure, num, &from_unit))
		return 0;
	if((from_unit == unit_err) || (to_unit == unit_err))
		return 0;
	if(!x2x(num, from_unit, to_unit))
		return 0;
	return 1;
}


// value and unit must be in the same argument
int main(int argc, char *argv[])
{
	pname = argv[0];
	if((argc - 1) % 2 != 0) {
		fprintf(stderr, "Wrong number of arguments\n");
		usage(1);
	}

	long double num;
	enum unit_t u;
	for(int i = 1; i < argc; i += 2) {
		if(!convert(argv[i], argv[i + 1], &num))
			continue;
		printf("%s->%s: %Lf\n", argv[i], argv[i + 1], num);
	}

	return 0;
}
