pub struct Mutant {
    pub identifier: String,
}

impl std::fmt::Debug for Mutant {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Mutant")
            .field("identifier", &self.identifier)
            .finish()
    }
}

impl std::hash::Hash for Mutant {
    fn hash<H: std::hash::Hasher>(&self, state: &mut H) {
        self.identifier.hash(state);
    }
}

impl PartialEq for Mutant {
    fn eq(&self, other: &Self) -> bool {
        self.identifier == other.identifier
    }
}

impl Eq for Mutant {}
