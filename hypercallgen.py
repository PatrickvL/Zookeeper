from yaml import load

calls = load(file('hypercalls.yaml'))

nhc = file('NightBeliever/Hypercall.cpp', 'w')
print >>nhc, '// Automatically generated by hypercallgen.py'
print >>nhc, '#include "NightBeliever.hpp"'
print >>nhc, '#include "../Hypercalls.hpp"'
print >>nhc

nhh = file('NightBeliever/Hypercall.hpp', 'w')
print >>nhh, '// Automatically generated by hypercallgen.py'
print >>nhh, '#pragma once'
print >>nhh, '#include "NightBeliever.hpp"'
print >>nhh, '#include "HypercallSupport.hpp"'
print >>nhh

zhc = file('HypercallDispatch.cpp', 'w')
print >>zhc, '// Automatically generated by hypercallgen.py'
print >>zhc, '#include "Zookeeper.hpp"'
print >>zhc
print >>zhc, 'void hypercall_dispatch(uint32_t call, uint32_t addr) {'
print >>zhc, '\tswitch(call) {'

zhh = file('Hypercalls.hpp', 'w')
print >>zhh, '// Automatically generated by hypercallgen.py'
print >>zhh, '#pragma once'
print >>zhh

zhl = file('Hypercall.hpp', 'w')
print >>zhl, '// Automatically generated by hypercallgen.py'
print >>zhl, '#pragma once'
print >>zhl, '#include "Zookeeper.hpp"'
print >>zhl, '#include "Hypercalls.hpp"'
print >>zhl
print >>zhl, 'void hypercall_dispatch(uint32_t call, uint32_t addr);'
print >>zhl
print >>zhl, 'class Hypercall {'
print >>zhl, 'public:'

def typemap(type):
	if '*' in type or type == 'thread_ep_t':
		return 'uint32_t'
	else:
		return type

def cast(a, b):
	if a == b:
		return ''
	return '(%s) ' % b

for i, (name, args) in enumerate(calls.items()):
	args = args if args is not None else []
	ret = ''
	nret = ''
	rettype = 'void'
	if args and 'return' in args[0]:
		rettype = args[0]['return']
		ret = 'auto ret = %s' % (cast(typemap(rettype), 'uint32_t'))
		nret = 'return %s' % (cast('uint32_t', rettype))
		del args[0]
	args = [(an, at, typemap(at)) for x in args for (an, at) in x.items()]

	print >>zhc, '\t\tcase %i: { // %s' % (i, name)

	if len(args) > 1:
		print >>zhh, 'typedef struct hypercall_%s {' % name
		for arg, type, mapped in args:
			print >>zhh, '\t%s %s;' % (mapped, arg)
		print >>zhh, '} hypercall_%s_t;' % name
		print >>zhh

		print >>zhc, '\t\t\tauto sarg = box->cpu->read_memory<hypercall_%s_t>(addr);' % (name)
		print >>zhc, '\t\t\t%sbox->hypercall->%s(%s);' % (ret, name, ', '.join('sarg.' + arg for arg, _, _ in args))
		if rettype != 'void':
			print >>zhc, '\t\t\tbox->cpu->hv->reg(EAX, ret);'

		print >>zhl, '\t%s %s(%s);' % (typemap(rettype), name, ', '.join('%s %s' % (mapped, name) for name, _, mapped in args))

		print >>nhh, '%s %s(%s);' % (rettype, name, ', '.join(type for _, type, _ in args))

		print >>nhc, '%s %s(%s) {' % (rettype, name, ', '.join('%s %s' % (type, name) for name, type, _ in args))
		print >>nhc, '\tvolatile hypercall_%s_t _sarg;' % name
		for arg, type, mapped in args:
			print >>nhc, '\t_sarg.%s = %s%s;' % (arg, cast(type, mapped), arg)
		print >>nhc, '\t%svmcall(%i, (uint32_t) &_sarg);' % (nret, i)
		print >>nhc, '}'
		print >>nhc
	elif len(args) == 1:
		print >>zhc, '\t\t\t%sbox->hypercall->%s(%saddr);' % (ret, name, cast('uint32_t', args[0][2]))
		if rettype != 'void':
			print >>zhc, '\t\t\tbox->cpu->hv->reg(EAX, ret);'

		print >>zhl, '\t%s %s(%s %s);' % (typemap(rettype), name, args[0][2], args[0][0])

		print >>nhh, '%s %s(%s);' % (rettype, name, args[0][1])

		print >>nhc, '%s %s(%s arg) {' % (rettype, name, args[0][1])
		print >>nhc, '\t%svmcall(%i, %sarg);' % (nret, i, cast(args[0][1], 'uint32_t'))
		print >>nhc, '}'
		print >>nhc
	else:
		print >>zhc, '\t\t\t%sbox->hypercall->%s();' % (ret, name)
		if rettype != 'void':
			print >>zhc, '\t\t\tbox->cpu->hv->reg(EAX, ret);'

		print >>zhl, '\t%s %s();' % (typemap(rettype), name)

		print >>nhh, '%s %s();' % (rettype, name)

		print >>nhc, '%s %s() {' % (rettype, name)
		print >>nhc, '\t%svmcall(%i, NULL);' % (nret, i)
		print >>nhc, '}'
		print >>nhc

	print >>zhc, '\t\t\tbreak;'
	print >>zhc, '\t\t}'

print >>zhc, '\t}'
print >>zhc, '}'
print >>zhc

print >>zhl, '};'
print >>zhl
