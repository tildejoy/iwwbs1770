/*
 * bg_print_conf.c
 *
 * Copyright (C) 2014 Peter Belkner <pbelkner@users.sf.net>
 * Copyright (C) 2019 Tilde Joy <tilde@ultros.pro>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2.0 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301  USA
 */
#include <bg.h>

///////////////////////////////////////////////////////////////////////////////
static double bg_print_conf_norm(bg_tree_t *tree)
{
  return tree->param->preamp+tree->param->norm;
}

////////
static double bg_print_conf_momentary_mean(bg_tree_t *tree)
{

  return lib1770_stats_get_mean(tree->stats.momentary,
      tree->param->momentary.mean.gate);
}

static double bg_print_conf_momentary_mean_relative(bg_tree_t *tree)
{
  double norm=bg_print_conf_norm(tree);
  double mean=bg_print_conf_momentary_mean(tree);

  return norm-mean;
}

////////
static double bg_print_conf_momentary_maximum(bg_tree_t *tree)
{
  return lib1770_stats_get_max(tree->stats.momentary);
}

static double bg_print_conf_momentary_maximum_relative(bg_tree_t *tree)
{
  double norm=bg_print_conf_norm(tree);
  double mean=bg_print_conf_momentary_maximum(tree);

  return norm-mean;
}

////////
static double bg_print_conf_momentary_range(bg_tree_t *tree)
{
  return lib1770_stats_get_range(tree->stats.momentary,
      tree->param->momentary.range.gate,
      tree->param->momentary.range.lower_bound,
      tree->param->momentary.range.upper_bound);
}

////////
static double bg_print_conf_shortterm_mean(bg_tree_t *tree)
{

  return lib1770_stats_get_mean(tree->stats.shortterm,
      tree->param->shortterm.mean.gate);
}

static double bg_print_conf_shortterm_mean_relative(bg_tree_t *tree)
{
  double norm=bg_print_conf_norm(tree);
  double mean=bg_print_conf_shortterm_mean(tree);

  return norm-mean;
}

////////
static double bg_print_conf_shortterm_maximum(bg_tree_t *tree)
{
  return lib1770_stats_get_max(tree->stats.shortterm);
}

static double bg_print_conf_shortterm_maximum_relative(bg_tree_t *tree)
{
  double norm=bg_print_conf_norm(tree);
  double mean=bg_print_conf_shortterm_maximum(tree);

  return norm-mean;
}

////////
static double bg_print_conf_shortterm_range(bg_tree_t *tree)
{
  return lib1770_stats_get_range(tree->stats.shortterm,
      tree->param->shortterm.range.gate,
      tree->param->momentary.range.lower_bound,
      tree->param->shortterm.range.upper_bound);
}

////////
static double bg_print_conf_samplepeak_absolute(bg_tree_t *tree)
{
  return tree->stats.samplepeak;
}

static double bg_print_conf_samplepeak_relative(bg_tree_t *tree)
{
  return LIB1770_Q2DB(tree->stats.samplepeak);
}

////////
static double bg_print_conf_truepeak_absolute(bg_tree_t *tree)
{
  return tree->stats.truepeak;
}

static double bg_print_conf_truepeak_relative(bg_tree_t *tree)
{
  return LIB1770_Q2DB(tree->stats.truepeak);
}

///////////////////////////////////////////////////////////////////////////////
static const char *bg_print_conf_unit_lum(bg_tree_t *tree)
{
  return tree->param->unit->n.lu;
}

#if defined (_WIN32) // [
static const wchar_t *bg_print_conf_unit_luw(bg_tree_t *tree)
{
  return tree->param->unit->w.lu;
}
#endif // ]

////////
static const char *bg_print_conf_unit_lram(bg_tree_t *tree)
{
  return tree->param->unit->n.lra;
}

#if defined (_WIN32) // [
static const wchar_t *bg_print_conf_unit_lraw(bg_tree_t *tree)
{
  return tree->param->unit->w.lra;
}
#endif // ]

////////
static const char *bg_print_conf_unit_spm(bg_tree_t *tree)
{
  return tree->param->unit->n.sp;
}

#if defined (_WIN32) // [
static const wchar_t *bg_print_conf_unit_spw(bg_tree_t *tree)
{
  return tree->param->unit->w.sp;
}
#endif // ]

////////
static const char *bg_print_conf_unit_tpm(bg_tree_t *tree)
{
  return tree->param->unit->n.tp;
}

#if defined (_WIN32) // [
static const wchar_t *bg_print_conf_unit_tpw(bg_tree_t *tree)
{
  return tree->param->unit->w.tp;
}
#endif // ]

////////
#if defined (_WIN32) // [
#define BG_CONF(agg,l,fc,u,uc,fx,c,v1,v2) { \
  .aggregate=agg, \
  .w.label=L##l, \
  .w.format.classic=L##fc, \
  .w.format.xml=L##fx, \
  .w.unit=bg_print_conf_unit_##u##w, \
  .n.label=l, \
  .n.format.classic=fc, \
  .n.format.xml=fx, \
  .n.unit=bg_print_conf_unit_##u##m, \
  .unitc=uc, \
  .argc=c, \
  .argv[0]=v1, \
  .argv[1]=v2 \
}
#else // ] [
#define BG_CONF(agg,l,fc,u,uc,fx,c,v1,v2) { \
  .aggregate=agg, \
  .n.label=l, \
  .n.format.classic=fc, \
  .n.format.xml=fx, \
  .n.unit=bg_print_conf_unit_##u##m, \
  .unitc=uc, \
  .argc=c, \
  .argv[0]=v1, \
  .argv[1]=v2 \
}
#endif // ]

#define BG_CONF1(agg,l,fc,u,uc,fx,v1) \
    BG_CONF(agg,l,fc,u,uc,fx,1,v1,NULL)
#define BG_CONF2(agg,l,fc,u,uc,fx,v1,v2) \
    BG_CONF(agg,l,fc,u,uc,fx,2,v1,v2)

bg_print_conf_t bg_print_conf[BG_FLAGS_AGG_MAX_OFFSET]={
  BG_CONF2(BG_FLAGS_AGG_MOMENTARY_MEAN,
      "integrated (momentary mean)",
      "  %%%ds: %%.2f %%sFS / %%.2f %%s\n",
      lu,
      2,
      "<integrated %sfs=\"%.2f\" %s=\"%.2f\"/>\n",
      bg_print_conf_momentary_mean,
      bg_print_conf_momentary_mean_relative),
  BG_CONF2(BG_FLAGS_AGG_MOMENTARY_MAXIMUM,
      "momentary maximum",
      "  %%%ds: %%.2f %%sFS / %%.2f %%s\n",
      lu,
      2,
      "<momentary %sfs=\"%.2f\" %s=\"%.2f\"/>\n",
      bg_print_conf_momentary_maximum,
      bg_print_conf_momentary_maximum_relative),
  BG_CONF1(BG_FLAGS_AGG_MOMENTARY_RANGE,
      "momentary loudness range",
      "  %%%ds: %%.2f %%s\n",
      lra,
      1,
      "<momentary-range %s=\"%.2f\"/>\n",
      bg_print_conf_momentary_range),
  BG_CONF2(BG_FLAGS_AGG_SHORTTERM_MEAN,
      "shortterm mean",
      "  %%%ds: %%.2f %%sFS / %%.2f %%s\n",
      lu,
      2,
      "<shortterm-mean %sfs=\"%.2f\" %s=\"%.2f\"/>\n",
      bg_print_conf_shortterm_mean,
      bg_print_conf_shortterm_mean_relative),
  BG_CONF2(BG_FLAGS_AGG_SHORTTERM_MAXIMUM,
      "shortterm maximum",
      "  %%%ds: %%.2f %%sFS / %%.2f %%s\n",
      lu,
      2,
      "<shortterm-maximum %sfs=\"%.2f\" %s=\"%.2f\"/>\n",
      bg_print_conf_shortterm_maximum,
      bg_print_conf_shortterm_maximum_relative),
  BG_CONF1(BG_FLAGS_AGG_SHORTTERM_RANGE,
      "(shortterm) loudness range",
      "  %%%ds: %%.2f %%s\n",
      lra,
      1,
      "<range %s=\"%.2f\"/>\n",
      bg_print_conf_shortterm_range),
  BG_CONF2(BG_FLAGS_AGG_SAMPLEPEAK,
      "sample peak (relative/absolute)",
      "  %%%ds: %%.2f %%sFS / %%f\n",
      sp,
      1,
      "<sample-peak %sfs=\"%.2f\" amplitude=\"%.2f\"/>\n",
      bg_print_conf_samplepeak_relative,
      bg_print_conf_samplepeak_absolute),
  BG_CONF2(BG_FLAGS_AGG_TRUEPEAK,
      "truepeak (relative/absolute)",
      "  %%%ds: %%.2f %%sFS / %%f\n",
      tp,
      1,
      "<truepeak %sfs=\"%.2f\" amplitude=\"%.2f\"/>\n",
      bg_print_conf_truepeak_relative,
      bg_print_conf_truepeak_absolute),
};
