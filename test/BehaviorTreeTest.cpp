#include <gtest/gtest.h>

#include "BehaviorTree.hpp"

namespace {

// The door example from the Phase 4 handout (teste.cpp).
BehaviorTree doorTree() {
  return BehaviorTree(Selector(
      Sequence(Condition("Passou"), Action("FechePorta")),
      Sequence(Condition("NaPorta"), Condition("PortaAberta"), Action("Passe")),
      Sequence(Condition("NaPorta"), Condition("PortaDestrancada"),
               Condition("PortaFechada"), Action("AbrirPorta")),
      Sequence(Condition("NaPorta"), Condition("PortaTrancada"),
               Action("DestranquePorta")),
      Action("CaminhePorta")));
}

} // namespace

TEST(BehaviorTree, ConditionNeedsNonZeroFact) {
  ConditionNode c("Enemy");
  EXPECT_FALSE(c.tick({}).success);
  EXPECT_FALSE(c.tick({{"Enemy", 0}}).success);
  EXPECT_TRUE(c.tick({{"Enemy", 1}}).success);
  EXPECT_TRUE(c.tick({{"Enemy", -3}}).success);
  EXPECT_TRUE(c.tick({{"Enemy", 1}}).command.empty());
}

TEST(BehaviorTree, ActionAlwaysSucceedsWithCommand) {
  ActionNode a("Jump");
  BTResult r = a.tick({});
  EXPECT_TRUE(r.success);
  EXPECT_EQ(r.command, "Jump");
}

TEST(BehaviorTree, SequenceFailsAtFirstFailingChild) {
  auto seq = Sequence(Action("A"), Condition("X"), Action("B"));
  BTResult r = seq->tick({});
  EXPECT_FALSE(r.success);
  EXPECT_TRUE(r.command.empty());
}

TEST(BehaviorTree, SequenceCarriesLastNonEmptyCommand) {
  auto seq = Sequence(Action("A"), Action("B"), Condition("X"));
  BTResult r = seq->tick({{"X", 1}});
  EXPECT_TRUE(r.success);
  EXPECT_EQ(r.command, "B");
}

TEST(BehaviorTree, EmptySequenceSucceedsAndEmptySelectorFails) {
  EXPECT_TRUE(SequenceNode({}).tick({}).success);
  EXPECT_FALSE(SelectorNode({}).tick({}).success);
}

TEST(BehaviorTree, SelectorTakesFirstSuccess) {
  auto sel = Selector(Sequence(Condition("X"), Action("A")), Action("B"),
                      Action("C"));
  EXPECT_EQ(sel->tick({}).command, "B");
  EXPECT_EQ(sel->tick({{"X", 1}}).command, "A");
}

TEST(BehaviorTree, FailedRootYieldsEmptyCommand) {
  BehaviorTree tree(Sequence(Condition("X"), Action("A")));
  EXPECT_EQ(tree.tick({}), "");
  EXPECT_EQ(tree.tick({{"X", 1}}), "A");
}

TEST(BehaviorTree, DoorExample) {
  const BehaviorTree tree = doorTree();
  EXPECT_EQ(tree.tick({{"NaPorta", 1}, {"PortaTrancada", 1}, {"PortaFechada", 1}}),
            "DestranquePorta");
  EXPECT_EQ(tree.tick({{"NaPorta", 1}, {"PortaDestrancada", 1}, {"PortaFechada", 1}}),
            "AbrirPorta");
  EXPECT_EQ(tree.tick({{"NaPorta", 1}, {"PortaAberta", 1}}), "Passe");
  EXPECT_EQ(tree.tick({{"Passou", 1}}), "FechePorta");
  EXPECT_EQ(tree.tick({}), "CaminhePorta");
}

TEST(BehaviorTree, PatrolExample) {
  // The back-and-forth patrol from the Phase 4 handout (teste2.cpp).
  const BehaviorTree tree(Selector(
      Sequence(Condition("ChegouPontoA"), Action("InverterDirecao")),
      Sequence(Condition("ChegouPontoB"), Action("InverterDirecao")),
      Sequence(Condition("IndoParaA"), Action("AndeParaA")),
      Sequence(Condition("IndoParaB"), Action("AndeParaB"))));

  EXPECT_EQ(tree.tick({{"IndoParaA", 1}}), "AndeParaA");
  EXPECT_EQ(tree.tick({{"IndoParaB", 1}}), "AndeParaB");
  EXPECT_EQ(tree.tick({{"IndoParaA", 1}, {"ChegouPontoA", 1}}), "InverterDirecao");
  EXPECT_EQ(tree.tick({}), "");
}
