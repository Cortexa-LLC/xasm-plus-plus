/**
 * @file atom_visitor.h
 * @brief IAtomVisitor pattern for atom dispatch
 *
 * Replaces the switch/dynamic_pointer_cast dispatch chains in EncodeInstructions
 * and RefixupDataAtoms with a proper Visitor pattern.  Each pass implements
 * IAtomVisitor; atom subclasses call back via Accept().
 *
 * Adding a new atom type: add a Visit() overload here, and an Accept() override
 * to the new subclass in atom.h — no switch statement needs updating.
 */

#pragma once

// Forward-declare all atom types to avoid pulling in the full atom.h here
// (atom.h will #include atom_visitor.h, so we must not create a cycle).
namespace xasm {

class Atom;
class LabelAtom;
class InstructionAtom;
class CpuModeAtom;
class MxAtom;
class DataAtom;
class SpaceAtom;
class AlignAtom;
class OrgAtom;
class DummyOrgAtom;
class PhaseAtom;
class ListingControlAtom;
class EquateAtom;

/**
 * @brief Visitor interface for all Atom subclasses.
 *
 * Implement this interface to write a pass over the atom list without
 * a switch statement.  Provide a no-op default implementation so
 * visitors only override the atom types they care about.
 */
class IAtomVisitor {
 public:
  virtual ~IAtomVisitor() = default;

  virtual void Visit(LabelAtom&) {}

  virtual void Visit(InstructionAtom&) {}

  virtual void Visit(CpuModeAtom&) {}

  virtual void Visit(MxAtom&) {}

  virtual void Visit(DataAtom&) {}

  virtual void Visit(SpaceAtom&) {}

  virtual void Visit(AlignAtom&) {}

  virtual void Visit(OrgAtom&) {}

  virtual void Visit(DummyOrgAtom&) {}

  virtual void Visit(PhaseAtom&) {}

  virtual void Visit(ListingControlAtom&) {}

  virtual void Visit(EquateAtom&) {}
};

}  // namespace xasm
