/*
  This file is part of the SC Library.
  The SC library provides support for parallel scientific applications.

  Copyright (C) 2009 Carsten Burstedde, Lucas Wilcox.

  The SC Library is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  The SC Library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with the SC Library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "car.h"
#include "vehicle.h"

const char         *car_type = "car";

static int
is_type_fn (sc_object_t * o, const char *type)
{
  SC_LDEBUG ("car is_type\n");

  return !strcmp (type, car_type) || !strcmp (type, vehicle_type);
}

static void
copy_fn (sc_object_t * o, sc_object_t * c)
{
  const Car          *car_o = car_get_data (o, 1);
  Car                *car_c = car_get_data (c, 0);

  SC_LDEBUG ("car copy\n");

  memcpy (car_c, car_o, sizeof (Car));
}

static void
initialize_fn (sc_object_t * o, sc_keyvalue_t * args)
{
  Car                *car = car_get_data (o, 0);

  SC_LDEBUG ("car initialize\n");

  car->speed = 0;
  car->wheelsize = 0.;

  if (args != NULL) {
    car->wheelsize = (float) sc_keyvalue_get_double (args, "wheelsize", 0.);
    SC_ASSERT (car->wheelsize > 0.);
  }
}

static void
write_fn (sc_object_t * o, sc_object_t * m, FILE * out)
{
  Car                *car = car_get_data (o, 1);

  fprintf (out, "Car (wheel size %f) speeds at %f km/h\n",
           car->wheelsize, car->speed);
}

static float
wheelsize_fn (sc_object_t * o, sc_object_t * m)
{
  Car                *car = car_get_data (o, 1);

  SC_LDEBUG ("car wheelsize\n");

  return car->wheelsize;
}

static void
accelerate_fn (sc_object_t * o, sc_object_t * m)
{
  Car                *car = car_get_data (o, 1);
  CarKlass           *car_klass;

  SC_LDEBUG ("car accelerate\n");

  car->speed += 10;

  car_klass = car_get_klass_data (m, 1);
  ++car_klass->repairs;
}

sc_object_t        *
car_klass_new (sc_object_t * d)
{
  int                 a1, a2, a3, a4, a5, a6;
  sc_object_t        *o;
  CarKlass           *car_klass;

  SC_ASSERT (d != NULL);
  SC_ASSERT (sc_object_is_type (d, sc_object_type));

  o = sc_object_alloc ();
  sc_object_delegate_push (o, d);

  a1 = sc_object_method_register (o, (sc_object_method_t) sc_object_is_type,
                                  (sc_object_method_t) is_type_fn);
  a2 = sc_object_method_register (o, (sc_object_method_t) sc_object_copy,
                                  (sc_object_method_t) copy_fn);
  a3 =
    sc_object_method_register (o, (sc_object_method_t) sc_object_initialize,
                               (sc_object_method_t) initialize_fn);
  a4 =
    sc_object_method_register (o, (sc_object_method_t) sc_object_write,
                               (sc_object_method_t) write_fn);
  a5 =
    sc_object_method_register (o, (sc_object_method_t) car_wheelsize,
                               (sc_object_method_t) wheelsize_fn);
  a6 =
    sc_object_method_register (o, (sc_object_method_t) vehicle_accelerate,
                               (sc_object_method_t) accelerate_fn);
  SC_ASSERT (a1 && a2 && a3 && a4 && a5 && a6);

  sc_object_initialize (o, NULL);
  car_klass = car_get_klass_data (o, 0);
  car_klass->repairs = 0;

  return o;
}

sc_object_t        *
car_new (sc_object_t * d, float wheelsize)
{
  return sc_object_new_from_klassf (d, "g:wheelsize", (double) wheelsize,
                                    NULL);
}

Car                *
car_get_data (sc_object_t * o, int exists)
{
  SC_ASSERT (sc_object_is_type (o, car_type));

  return (Car *) sc_object_get_data (o, (sc_object_method_t) car_get_data,
                                     0, exists, sizeof (Car));
}

CarKlass           *
car_get_klass_data (sc_object_t * o, int exists)
{
  SC_ASSERT (sc_object_is_type (o, car_type));

  return (CarKlass *) sc_object_get_data (o, (sc_object_method_t)
                                          car_get_klass_data, 0, exists,
                                          sizeof (Car));
}

float
car_wheelsize (sc_object_t * o)
{
  sc_object_method_t  oinmi;
  sc_object_t        *m;

  SC_ASSERT (sc_object_is_type (o, car_type));

  oinmi =
    sc_object_method_search (o, (sc_object_method_t) car_wheelsize, 0, &m);
  SC_ASSERT (oinmi != NULL);

  return ((float (*)(sc_object_t *, sc_object_t *)) oinmi) (o, m);
}
