#include "../Structures.hpp"
#include "zensim/Logger.hpp"
#include "zensim/geometry/PoissonDisk.hpp"
#include "zensim/geometry/VdbLevelSet.h"
#include "zensim/geometry/VdbSampler.h"
#include "zensim/omp/execution/ExecutionPolicy.hpp"
#include <zeno/VDBGrid.h>
#include <zeno/types/NumericObject.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/StringObject.h>

namespace zeno {

struct PoissonDiskSample : INode {
  void apply() override {
    using namespace zs;
    fmt::print(fg(fmt::color::green), "begin executing PoissonDiskSample\n");
    auto ls = std::make_shared<ZenoLevelSet>();

    zs::OpenVDBStruct gridPtr{};
    if (has_input<VDBFloatGrid>("VDBGrid"))
      gridPtr = get_input<VDBFloatGrid>("VDBGrid")->m_grid;
    else
      gridPtr =
          zs::load_floatgrid_from_vdb_file(get_param<std::string>("path"));

    // auto spls = zs::convert_floatgrid_to_sparse_levelset(
    //    gridPtr, {zs::memsrc_e::host, -1});
    auto dx = get_input2<float>("dx");
#if 0
    auto sampled = zs::sample_from_levelset(
        zs::proxy<zs::execspace_e::openmp>(spls), dx, get_input2<float>("ppc"));
#else
    auto sampled =
        zs::sample_from_levelset(gridPtr, dx, get_input2<float>("ppc"));
#endif

    auto prim = std::make_shared<PrimitiveObject>();
    prim->resize(sampled.size());
    auto &pos = prim->attr<vec3f>("pos");
    auto &vel = prim->add_attr<vec3f>("vel");
    // auto &nrm = prim->add_attr<vec3f>("nrm");

    /// compute default normal
    auto ompExec = zs::omp_exec();
#if 0
    const auto calcNormal = [spls = proxy<zs::execspace_e::host>(spls),
                             eps = dx](const vec3f &x_) {
      zs::vec<float, 3> x{x_[0], x_[1], x_[2]}, diff{};
      /// compute a local partial derivative
      for (int i = 0; i != 3; i++) {
        auto v1 = x;
        auto v2 = x;
        v1[i] = x[i] + eps;
        v2[i] = x[i] - eps;
        diff[i] = (spls.getSignedDistance(v1) - spls.getSignedDistance(v2)) /
                  (eps + eps);
      }
      if (math::near_zero(diff.l2NormSqr()))
        return vec3f{0, 0, 0};
      auto r = diff.normalized();
      return vec3f{r[0], r[1], r[2]};
    };
#endif
    ompExec(zs::range(sampled.size()), [&sampled, &pos, &vel](size_t pi) {
      pos[pi] = sampled[pi];
      vel[pi] = vec3f{0, 0, 0};
      // nrm[pi] = calcNormal(pos[pi]);
    });

    fmt::print(fg(fmt::color::cyan), "done executing PoissonDiskSample\n");
    set_output("prim", std::move(prim));
  }
};
ZENDEFNODE(PoissonDiskSample,
           {
               {"VDBGrid", {"float", "dx", "0.1"}, {"float", "ppc", "8"}},
               {"prim"},
               {{"string", "path", ""}},
               {"MPM"},
           });

struct ZSPoissonDiskSample : INode {
  void apply() override {
    using namespace zs;
    fmt::print(fg(fmt::color::green), "begin executing ZSPoissonDiskSample\n");
    auto prim = std::make_shared<PrimitiveObject>();
    std::vector<std::array<float, 3>> sampled;

    auto zsfield = get_input<ZenoLevelSet>("ZSLevelSet");
    auto dx = get_input2<float>("dx");
    auto ppc = get_input2<float>("ppc");

    match([&prim, &sampled, dx, ppc, this](auto &ls) {
      using LsT = RM_CVREF_T(ls);
      if constexpr (is_same_v<LsT, typename ZenoLevelSet::basic_ls_t>) {
        auto &field = ls._ls;
        match(
            [&sampled = sampled, dx = dx, ppc = ppc](auto &lsPtr)
                -> std::enable_if_t<
                    is_spls_v<typename RM_CVREF_T(lsPtr)::element_type>> {
              const auto &ls = *lsPtr;
              const auto &spls = ls.memspace() != memsrc_e::host
                                     ? ls.clone({memsrc_e::host, -1})
                                     : ls;
              sampled = zs::sample_from_levelset(
                  zs::proxy<zs::execspace_e::openmp>(spls), dx, ppc);
            },
            [](auto &lsPtr)
                -> std::enable_if_t<
                    !is_spls_v<typename RM_CVREF_T(lsPtr)::element_type>> {
              throw std::runtime_error(
                  fmt::format("levelset type [{}] not supported in sampling.",
                              zs::get_var_type_str(lsPtr)));
            })(field);
      } else if constexpr (is_same_v<
                               LsT,
                               typename ZenoLevelSet::const_sdf_vel_ls_t>) {
        match([&sampled = sampled, dx = dx, ppc = ppc](auto lsv) {
          sampled = zs::sample_from_levelset(SdfVelFieldView{lsv}, dx, ppc);
        })(ls.template getView<execspace_e::openmp>());
      } else if constexpr (is_same_v<
                               LsT,
                               typename ZenoLevelSet::const_transition_ls_t>) {
        match([&sampled = sampled, &ls, dx = dx, ppc = ppc](auto fieldPair) {
          sampled = zs::sample_from_levelset(
              TransitionLevelSetView{SdfVelFieldView{get<0>(fieldPair)},
                                     SdfVelFieldView{get<1>(fieldPair)},
                                     ls._stepDt, ls._alpha},
              dx, ppc);
        })(ls.template getView<execspace_e::openmp>());
      } else {
        throw std::runtime_error("unknown levelset type...");
      }
    })(zsfield->levelset);

    prim->resize(sampled.size());
    auto &pos = prim->attr<vec3f>("pos");
    auto &vel = prim->add_attr<vec3f>("vel");

    /// compute default normal
    auto ompExec = zs::omp_exec();
    ompExec(zs::range(sampled.size()), [&sampled, &pos, &vel](size_t pi) {
      pos[pi] = sampled[pi];
      vel[pi] = vec3f{0, 0, 0};
      // nrm[pi] = calcNormal(pos[pi]);
    });

    fmt::print(fg(fmt::color::cyan), "done executing ZSPoissonDiskSample\n");
    set_output("prim", std::move(prim));
  }
};
ZENDEFNODE(ZSPoissonDiskSample,
           {
               {"ZSLevelSet", {"float", "dx", "0.1"}, {"float", "ppc", "8"}},
               {"prim"},
               {{"string", "path", ""}},
               {"MPM"},
           });

struct ScalePrimitiveAlongNormal : zeno::INode {
  virtual void apply() override {
    auto prim = get_input<PrimitiveObject>("prim");
    set_output("prim", get_input("prim"));

    if (!prim->has_attr("nrm"))
      return;

    auto &nrm = prim->attr<zeno::vec3f>("nrm");
    auto &pos = prim->attr<zeno::vec3f>("pos");
    auto dis = get_input2<float>("dis");

    auto ompExec = zs::omp_exec();
    ompExec(zs::range(pos.size()),
            [&](size_t pi) { pos[pi] += nrm[pi] * dis; });
  }
};

ZENDEFNODE(ScalePrimitiveAlongNormal, {
                                          {"prim", {"float", "dis", "0"}},
                                          {"prim"},
                                          {},
                                          {"primitive"},
                                      });

struct ComputePrimitiveSequenceVelocity : zeno::INode {
  virtual void apply() override {
    auto prim0 = get_input<PrimitiveObject>("prim0");
    auto prim1 = get_input<PrimitiveObject>("prim1");

    if (prim0->size() != prim1->size())
      throw std::runtime_error(
          "consecutive sequence objs with different topo!");

    auto &p0 = prim0->attr<zeno::vec3f>("pos");
    auto &p1 = prim1->attr<zeno::vec3f>("pos");
    auto &v0 = prim0->add_attr<zeno::vec3f>("vel");
    auto &v1 = prim1->add_attr<zeno::vec3f>("vel");

    auto ompExec = zs::omp_exec();
    ompExec(zs::range(p0.size()), [&, dt = get_input2<float>("dt")](size_t pi) {
      v0[pi] = (p1[pi] - p0[pi]) / dt;
      v1[pi] = vec3f{0, 0, 0};
    });
  }
};

ZENDEFNODE(ComputePrimitiveSequenceVelocity,
           {
               {"prim0", "prim1", {"float", "dt", "1"}},
               {},
               {},
               {"primitive"},
           });

struct ToZSLevelSet : INode {
  void apply() override {
    fmt::print(fg(fmt::color::green), "begin executing ToZSLevelSet\n");
    auto ls = std::make_shared<ZenoLevelSet>();

    using basic_ls_t = typename ZenoLevelSet::basic_ls_t;

    if (has_input<VDBFloatGrid>("VDBGrid")) {
      // pass in FloatGrid::Ptr
      zs::OpenVDBStruct gridPtr = get_input<VDBFloatGrid>("VDBGrid")->m_grid;
      ls->getLevelSet() = basic_ls_t{zs::convert_floatgrid_to_sparse_levelset(
          gridPtr, zs::MemoryProperty{zs::memsrc_e::um, 0})};
    } else if (has_input<VDBFloat3Grid>("VDBGrid")) {
      // pass in FloatGrid::Ptr
      zs::OpenVDBStruct gridPtr = get_input<VDBFloat3Grid>("VDBGrid")->m_grid;
      ls->getLevelSet() =
          basic_ls_t{zs::convert_vec3fgrid_to_sparse_staggered_grid(
              gridPtr, zs::MemoryProperty{zs::memsrc_e::um, 0})};
    } else {
      auto path = get_param<std::string>("path");
      auto gridPtr = zs::load_vec3fgrid_from_vdb_file(path);
      ls->getLevelSet() = basic_ls_t{zs::convert_vec3fgrid_to_sparse_levelset(
          gridPtr, zs::MemoryProperty{zs::memsrc_e::um, 0})};
    }

    fmt::print(fg(fmt::color::cyan), "done executing ToZSLevelSet\n");
    set_output("ZSLevelSet", std::move(ls));
  }
};
ZENDEFNODE(ToZSLevelSet, {
                             {"VDBGrid"},
                             {"ZSLevelSet"},
                             {{"string", "path", ""}},
                             {"MPM"},
                         });

struct ComposeSdfVelField : INode {
  void apply() override {
    fmt::print(fg(fmt::color::green), "begin executing ComposeSdfVelField\n");
    auto ls = std::make_shared<ZenoLevelSet>();

    std::shared_ptr<ZenoLevelSet> sdfLsPtr{};
    std::shared_ptr<ZenoLevelSet> velLsPtr{};

    using const_sdf_vel_ls_t = typename ZenoLevelSet::const_sdf_vel_ls_t;

    if (has_input<ZenoLevelSet>("ZSSdfField")) {
      sdfLsPtr = get_input<ZenoLevelSet>("ZSSdfField");
    }
    if (has_input<ZenoLevelSet>("ZSVelField")) {
      velLsPtr = get_input<ZenoLevelSet>("ZSVelField");
    }
    if (velLsPtr) {
      if (!sdfLsPtr->holdsBasicLevelSet() || !velLsPtr->holdsBasicLevelSet()) {
        auto msg = fmt::format("sdfField is {}a basic levelset {} and velField "
                               "is {}a basic levelset.\n",
                               sdfLsPtr->holdsBasicLevelSet() ? "" : "not ",
                               velLsPtr->holdsBasicLevelSet() ? "" : "not ");
        throw std::runtime_error(msg);
      }
      ls->getLevelSet() = const_sdf_vel_ls_t{sdfLsPtr->getBasicLevelSet(),
                                             velLsPtr->getBasicLevelSet()};
    } else {
      if (!sdfLsPtr->holdsBasicLevelSet()) {
        auto msg = fmt::format("sdfField is {}a basic levelset.\n",
                               sdfLsPtr->holdsBasicLevelSet() ? "" : "not ");
        throw std::runtime_error(msg);
      }
      ls->getLevelSet() = const_sdf_vel_ls_t{sdfLsPtr->getBasicLevelSet()};
    }

    fmt::print(fg(fmt::color::cyan), "done executing ComposeSdfVelField\n");
    set_output("ZSLevelSet", std::move(ls));
  }
};
ZENDEFNODE(ComposeSdfVelField, {
                                   {"ZSSdfField", "ZSVelField"},
                                   {"ZSLevelSet"},
                                   {},
                                   {"MPM"},
                               });

struct EnqueueLevelSetSequence : INode {
  void apply() override {
    using namespace zs;
    fmt::print(fg(fmt::color::green),
               "begin executing EnqueueLevelSetSequence\n");

    using basic_ls_t = typename ZenoLevelSet::basic_ls_t;
    using const_sdf_vel_ls_t = typename ZenoLevelSet::const_sdf_vel_ls_t;
    using const_transition_ls_t = typename ZenoLevelSet::const_transition_ls_t;

    std::shared_ptr<ZenoLevelSet> zsls{};
    if (has_input<ZenoLevelSet>("ZSLevelSetSequence"))
      zsls = get_input<ZenoLevelSet>("ZSLevelSetSequence");
    else {
      zsls = std::make_shared<ZenoLevelSet>();
      zsls->levelset = const_transition_ls_t{};
    }
    auto &lsseq = zsls->getLevelSetSequence();

    if (has_input<ZenoLevelSet>("ZSLevelSet")) {
      auto &ls = get_input<ZenoLevelSet>("ZSLevelSet")->getLevelSet();
      match(
          [&lsseq](basic_ls_t &basicLs) {
            lsseq.push(const_sdf_vel_ls_t{basicLs});
          },
          [&lsseq](const_sdf_vel_ls_t &field) { // recommend
            lsseq.push(field); // also reset alpha in the meantime
          },
          [&lsseq](const_transition_ls_t &seq) {
            lsseq._fields.insert(lsseq._fields.end(), seq._fields.begin(),
                                 seq._fields.end());
          })(ls);
    }

    fmt::print(fg(fmt::color::cyan),
               "done executing EnqueueLevelSetSequence\n");
    set_output("ZSLevelSetSequence", std::move(zsls));
  }
};
ZENDEFNODE(EnqueueLevelSetSequence, {
                                        {"ZSLevelSetSequence", "ZSLevelSet"},
                                        {"ZSLevelSetSequence"},
                                        {},
                                        {"MPM"},
                                    });

struct MakeZSString : INode {
  void apply() override {
    auto n = std::make_shared<StringObject>();
    n->value = fmt::format(get_input<StringObject>("fmt_str")->get(),
                           get_input<NumericObject>("frameid")->get<int>(),
                           get_input<NumericObject>("stepid")->get<int>());
    set_output("str", std::move(n));
  }
};
ZENDEFNODE(MakeZSString, {
                             {"fmt_str", "frameid", "stepid"},
                             {"str"},
                             {},
                             {"SOP"},
                         });

/// update levelsetsequence state
struct UpdateLevelSetSequence : INode {
  void apply() override {
    using namespace zs;
    fmt::print(fg(fmt::color::green),
               "begin executing UpdateLevelSetSequence\n");

    using basic_ls_t = typename ZenoLevelSet::basic_ls_t;
    using const_sdf_vel_ls_t = typename ZenoLevelSet::const_sdf_vel_ls_t;
    using const_transition_ls_t = typename ZenoLevelSet::const_transition_ls_t;

    std::shared_ptr<ZenoLevelSet> zsls{};
    if (has_input<ZenoLevelSet>("ZSLevelSetSequence"))
      zsls = get_input<ZenoLevelSet>("ZSLevelSetSequence");
    else {
      zsls = std::make_shared<ZenoLevelSet>();
      zsls->getLevelSet() = const_transition_ls_t{};
    }
    auto &lsseq = zsls->getLevelSetSequence();

    if (has_input<NumericObject>("dt")) {
      auto stepDt = get_input<NumericObject>("dt")->get<float>();
      lsseq.setStepDt(stepDt);
      // fmt::print("\tdt: {}\n", lsseq._stepDt);
    }

    if (has_input<NumericObject>("alpha")) {
      auto alpha = get_input<NumericObject>("alpha")->get<float>();
      lsseq.advance(alpha);
      // fmt::print("\talpha: {}, accum: {}\n", alpha, lsseq._alpha);
    }

#if 0
    fmt::print("\t{} levelset in queue.\n", lsseq._fields.size());
    match([&lsseq](auto fieldPair) {
      auto lsseqv = TransitionLevelSetView{SdfVelFieldView{get<0>(fieldPair)},
                                           SdfVelFieldView{get<1>(fieldPair)},
                                           lsseq._stepDt, lsseq._alpha};
      fmt::print("ls_seq_view type: [{}]\n", get_var_type_str(lsseqv));
      auto printBox = [](std::string_view msg, auto &lsv) {
        auto [mi, ma] = lsv.getBoundingBox();
        fmt::print("[{}]: [{}, {}, {} ~ {}, {}, {}]\n", msg, mi[0], mi[1],
                   mi[2], ma[0], ma[1], ma[2]);
      };
      printBox("src", lsseqv._lsvSrc);
      printBox("dst", lsseqv._lsvDst);
      printBox("cur", lsseqv);
    })(lsseq.template getView<execspace_e::openmp>());
#endif

    fmt::print(fg(fmt::color::cyan), "done executing UpdateLevelSetSequence\n");
    set_output("ZSLevelSetSequence", std::move(zsls));
  }
};
ZENDEFNODE(UpdateLevelSetSequence, {
                                       {"ZSLevelSetSequence", "dt", "alpha"},
                                       {"ZSLevelSetSequence"},
                                       {},
                                       {"MPM"},
                                   });

struct ZSLevelSetToVDBGrid : INode {
  void apply() override {
    fmt::print(fg(fmt::color::green), "begin executing ZSLevelSetToVDBGrid\n");
    auto vdb = std::make_shared<VDBFloatGrid>();

    if (has_input<ZenoLevelSet>("ZSLevelSet")) {
      auto ls = get_input<ZenoLevelSet>("ZSLevelSet");
      if (ls->holdsBasicLevelSet()) {
        zs::match(
            [&vdb](auto &lsPtr)
                -> std::enable_if_t<
                    zs::is_spls_v<typename RM_CVREF_T(lsPtr)::element_type>> {
              using LsT = typename RM_CVREF_T(lsPtr)::element_type;
              vdb->m_grid = zs::convert_sparse_levelset_to_vdbgrid(*lsPtr)
                                .template as<openvdb::FloatGrid::Ptr>();
            },
            [](...) {})(ls->getBasicLevelSet()._ls);
      } else
        ZS_WARN("The current input levelset is not a sparse levelset!");
    }

    fmt::print(fg(fmt::color::cyan), "done executing ZSLevelSetToVDBGrid\n");
    set_output("VDBFloatGrid", std::move(vdb));
  }
};
ZENDEFNODE(ZSLevelSetToVDBGrid, {
                                    {"ZSLevelSet"},
                                    {"VDBFloatGrid"},
                                    {},
                                    {"MPM"},
                                });

} // namespace zeno